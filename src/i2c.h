/*
 * i2c.h
 *
 *  Created on: 28 de abr. de 2023
 *      Author: practiques
 */

#ifndef SRC_I2C_H_
#define SRC_I2C_H_
#include <stdio.h>
#include <stdbool.h>


typedef struct{
	uint16_t R;
	uint16_t G;
	uint16_t B;
} RGB;

typedef enum {ROJO, AZUL, VERDE, AMARILLO, BLANCO, NEGRO, MORADO, CIAN} Colores;
static const char *Color_list[] = {"ROJO", "AZUL", "VERDE", "AMARILLO", "BLANCO", "NEGRO", "MORADO", "CIAN"};

int _write(int file, const char *ptr, int len);
void BSP_I2C_Init(uint8_t addr);
bool I2C_WriteRegister(uint8_t reg, uint8_t data);
bool I2C_ReadRegister(uint8_t reg, uint8_t *val);
bool I2C_Test();
RGB ReadSensor();
void printColor(Colores color);
uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
#endif /* SRC_I2C_H_ */
