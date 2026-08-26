#ifndef I2C_H
#define I2C_H

#include "stm32f446xx.h"
#include "stdint.h"

#define I2C_TIMEOUT 100000U

void clock_init(void);

void gpio_init(void);

void i2c_init(void);

uint8_t I2C_Master_Start(void);

uint8_t I2C_Master_Send_Address(uint8_t address, uint8_t read);

uint8_t I2C_Master_Send_Data(uint8_t data);

uint8_t I2C_Master_Write(uint8_t address, uint8_t *data, uint8_t length);

uint8_t I2C_Master_Read(uint8_t address, uint8_t *buffer, uint8_t length);

void I2C_Master_Stop(void);

#endif