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

int _write(int file, const char *ptr, int len);
void BSP_I2C_Init(uint8_t addr);
bool I2C_WriteRegister(uint8_t reg, uint8_t data);
bool I2C_ReadRegister(uint8_t reg, uint8_t *val);
bool I2C_Test();

#endif /* SRC_I2C_H_ */
