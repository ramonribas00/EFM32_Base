/***************************************************************************//**
 * @file
 * @brief FreeRTOS Blink Demo for Energy Micro EFM32GG_STK3700 Starter Kit
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/*SCL -- SERIAL CLOCK --- PC5
 * SDA I2C DATA --- PC4
 *
*/
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "croutine.h"

#include "em_chip.h"
#include "bsp.h"
#include "bsp_trace.h"

#include "sleep.h"

#include "i2c.h"

#define STACK_SIZE_FOR_TASK    (configMINIMAL_STACK_SIZE + 10)
#define TASK_PRIORITY          (tskIDLE_PRIORITY + 1)
#define SFE_ISL29125			RGB_sensor


QueueHandle_t xQueue = NULL;
QueueHandle_t xQueue2 = NULL;

/* Structure with parameters for LedBlink */
typedef struct {
  /* Delay between blink of led */
  portTickType delay;
  /* Number of led */
  int          ledNo;
} TaskParams_t;

/***************************************************************************//**
 * @brief Simple task which is blinking led
 * @param *pParameters pointer to parameters passed to the function
 ******************************************************************************/
/*
static void LedBlink(void *pParameters)
{
  TaskParams_t     * pData = (TaskParams_t*) pParameters;
  const portTickType delay = pData->delay;
  for (;; ) {
    BSP_LedToggle(pData->ledNo);

    vTaskDelay(delay);
  }
}
*/

static void ReadRGB(void *pParameters)
{
  const portTickType delay = pdMS_TO_TICKS(1000);
  RGB values;
  for (;; ) {
	values = ReadSensor();
	xQueueSend(xQueue, (void *) &values, (TickType_t ) 0 );
    vTaskDelay(delay);
  }
}

static void processRGB(void *pParameters){
  RGB values;
  RGB valuesaux;


  Colores color;

  for (;; ) {
    if (xQueue != NULL){
      if (xQueueReceive(xQueue,(void *) &values, portMAX_DELAY) == pdPASS) {
    	valuesaux.R = map(values.R, 0, 65535, 0, 255);
    	valuesaux.G = map(values.G, 0, 65535, 0, 255);
    	valuesaux.B = map(values.B, 0, 65535, 0, 255);
        printf("Los valores de RGB son:  %d, %d, %d\n", valuesaux.R, valuesaux.G, valuesaux.B);

        color = processColor(valuesaux);

        //funcion que cambie los valores recibidos por el color en concreto a mostrar.

        xQueueSend(xQueue2, (void *) &color, (TickType_t ) 0 );
      }
      else{
        printf("No se ha recibido la cola correctamente.\n");
      }
    }
  }
}


static void showColor(void *pParameters)
{
	const portTickType delay = pdMS_TO_TICKS(1000);
	Colores color;
	  for (;; ) {
		  if(xQueueReceive(xQueue2, &( color), portMAX_DELAY) == pdPASS)
			  printColor(color);
	    vTaskDelay(delay);
	  }
}
/***************************************************************************//**
 * @brief  Main function
 ******************************************************************************/
int main(void)
{

  /* Chip errata */
  CHIP_Init();
  /* If first word of user data page is non-zero, enable Energy Profiler trace */
  BSP_TraceProfilerSetup();

  /* Initialize LED driver */
  BSP_LedsInit();
  /* Setting state of leds*/
  BSP_LedSet(0);
  BSP_LedSet(1);

  /* Initialize SLEEP driver, no callbacks are used */
  SLEEP_Init(NULL, NULL);
#if (configSLEEP_MODE < 3)
  /* do not let to sleep deeper than define */
  SLEEP_SleepBlockBegin((SLEEP_EnergyMode_t)(configSLEEP_MODE + 1));
#endif

  BSP_I2C_Init(0x88);
  xQueue = xQueueCreate(5, sizeof(RGB));
  xQueue2 = xQueueCreate(5, sizeof(Colores));
  /* Parameters value for taks*/
  static TaskParams_t parametersToTask1 = { 0, 0 };
  static TaskParams_t parametersToTask2 = { 0, 0};
  static RGB parametersToTask3 = { 0, 0, 0};
  /*Create two task for blinking leds*/
  xTaskCreate(processRGB, (const char *) "processRGB", STACK_SIZE_FOR_TASK, &parametersToTask1, TASK_PRIORITY, NULL);
  xTaskCreate(ReadRGB, (const char *) "ReadRGB", STACK_SIZE_FOR_TASK, &parametersToTask3, TASK_PRIORITY, NULL);
  xTaskCreate(showColor, (const char *) "showColor", STACK_SIZE_FOR_TASK, &parametersToTask2, TASK_PRIORITY, NULL);

  /*Start FreeRTOS Scheduler*/
  vTaskStartScheduler();

  return 0;
}
