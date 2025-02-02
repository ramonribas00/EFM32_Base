#include <stdio.h>
#include <stdbool.h>
#include "em_i2c.h"
#include "em_gpio.h"
#include "em_cmu.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include "i2c.h"


static uint8_t device_addr;

SemaphoreHandle_t xSemaphore = NULL;



int _write(int file, const char *ptr, int len) {
    int x;
    for (x = 0; x < len; x++) {
       ITM_SendChar (*ptr++);
    }
    return (len);
}

void BSP_I2C_Init(uint8_t addr) {

	I2C_Init_TypeDef i2cInit = I2C_INIT_DEFAULT;
	CMU_ClockEnable(cmuClock_I2C1, true);
	GPIO_PinModeSet(gpioPortC, 4, gpioModeWiredAnd, 1);
	GPIO_PinModeSet(gpioPortC, 5, gpioModeWiredAnd, 1);
	I2C1->ROUTE = I2C_ROUTE_SDAPEN |
	I2C_ROUTE_SCLPEN | I2C_ROUTE_LOCATION_LOC0;
	I2C_Init(I2C1, &i2cInit);

	vSemaphoreCreateBinary( xSemaphore );

	device_addr = addr;
	I2C_WriteRegister(0x01, 5);
	I2C_WriteRegister(0x08, 0);
}

/**
 * @brief Write register using default I2C bus
 * @param reg register to write
 * @param data data to write
 * @return true on success
 */
bool I2C_WriteRegister(uint8_t reg, uint8_t data) {

	xSemaphoreTake(xSemaphore, portMAX_DELAY);

	I2C_TransferReturn_TypeDef I2C_Status;
	bool ret_value = false;

	I2C_TransferSeq_TypeDef seq;
	uint8_t dataW[2];

	seq.addr = device_addr;
	seq.flags = I2C_FLAG_WRITE;

	/* Register to write: 0x67 ( INT_FLAT )*/
	dataW[0] = reg;
	dataW[1] = data;

	seq.buf[0].data = dataW;
	seq.buf[0].len = 2;
	I2C_Status = I2C_TransferInit(I2C1, &seq);

	while (I2C_Status == i2cTransferInProgress) {
		I2C_Status = I2C_Transfer(I2C1);
	}

	if (I2C_Status != i2cTransferDone) {
		ret_value = false;
	} else {
		ret_value = true;
	}
	xSemaphoreGive(xSemaphore);
	return ret_value;
}

/**
 * @brief Read register from I2C device
 * @param reg Register to read
 * @param val Value read
 * @return true on success
 */
bool I2C_ReadRegister(uint8_t reg, uint8_t *val) {
	xSemaphoreTake(xSemaphore, portMAX_DELAY);

	I2C_TransferReturn_TypeDef I2C_Status;
	I2C_TransferSeq_TypeDef seq;
	uint8_t data[2];

	seq.addr = device_addr;
	seq.flags = I2C_FLAG_WRITE_READ;

	seq.buf[0].data = &reg;
	seq.buf[0].len = 1;
	seq.buf[1].data = data;
	seq.buf[1].len = 1;

	I2C_Status = I2C_TransferInit(I2C1, &seq);

	while (I2C_Status == i2cTransferInProgress) {
		I2C_Status = I2C_Transfer(I2C1);
	}

	if (I2C_Status != i2cTransferDone) {
		xSemaphoreGive(xSemaphore);
		return false;
	}

	*val = data[0];
	xSemaphoreGive(xSemaphore);
	return true;
}

bool I2C_Test() {
	uint8_t data;



	printf("I2C: %02X\n", data);

	if (data == 0x7D) {
		return true;
	} else {
		return false;
	}

}

RGB ReadSensor() {
	uint8_t RdataHi=0, RdataLo;
	uint8_t GdataHi=0, GdataLo;
	uint8_t BdataHi=0, BdataLo;

	RGB ReadVal;
	uint8_t x=0;
	I2C_WriteRegister(0x02, 0);

	//I2C_WriteRegister(0x01, 2);
	//while(x!=34){I2C_ReadRegister(0x08, &x);}
	//printf("I2C x: %d\n", x);
	I2C_ReadRegister(0x0C, &RdataHi);
	I2C_ReadRegister(0x0B, &RdataLo);
	ReadVal.B = (uint16_t)((RdataHi << 8) | RdataLo);	//RED READ
	//printf("I2C R: %d\n", ReadVal.R);
	//I2C_WriteRegister(0x01, 1);

	//while(x!=18){I2C_ReadRegister(0x08, &x);}
	//printf("I2C x: %d\n", x);
	I2C_ReadRegister(0x0A, &GdataHi);
	I2C_ReadRegister(0x09, &GdataLo);
	ReadVal.R = (uint16_t)((GdataHi << 8) | GdataLo);	//GREEN READ
	//printf("I2C G: %d\n", ReadVal.G);

	//I2C_WriteRegister(0x01, 3);
	//while(x!=50){I2C_ReadRegister(0x08, &x);}
	//printf("I2C x: %d\n", x);
	I2C_ReadRegister(0x0E, &BdataHi);
	I2C_ReadRegister(0x0D, &BdataLo);
	ReadVal.G =(uint16_t)((BdataHi << 8) | BdataLo);	//BLUE READ
	//printf("I2C B: %d\n", ReadVal.B);


	printf("--------------------\n");

	//%02X
	return ReadVal;
}

Colores processColor(RGB rgb){
	uint8_t r, g, b;
	r=rgb.R/(uint8_t)128;
	g=rgb.G/(uint8_t)128;
	b=rgb.B/(uint8_t)128;
	//printf("Los valores de RGB son:  %d, %d, %d\n", r, g, b);
	Colores result;
	switch(r){
	case 0:		//VERDE-AZUL-NEGRO-CIAN
			if(g==0){	//NEGRO-AZUL
				if(b==0) result=5;
				else	result=1;
			}else{ //VERDE-CIAN
				if(b==0) result=2;
				else result=7;
			}
		break;
	case 1:		//ROJO-AMARILLO-MORADO-BLANCO
		if(g==0){	//ROJO-MORADO
			if(b==0) result=0;
			else	result=6;
		}else{ //AMARILLO-BLANCO
			if(b==0) result=3;
			else result=4;
		}
		break;
	default: printf("------------\n");
		break;
	}
	return result;
}

void printColor(Colores color){

	printf("I2C Color: %s\n", Color_list[color%8]);

}
uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
