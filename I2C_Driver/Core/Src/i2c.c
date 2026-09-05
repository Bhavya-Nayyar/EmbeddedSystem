#include "i2c.h"

void clock_init(void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

  (void)RCC->AHB1ENR;
  (void)RCC->APB1ENR;
}

void gpio_init(void) {
  GPIOB->MODER &= ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9);
  GPIOB->MODER |= (2U << GPIO_MODER_MODER8_Pos) | (2U << GPIO_MODER_MODER9_Pos);
  GPIOB->OTYPER |= GPIO_OTYPER_OT8 | GPIO_OTYPER_OT9;
  GPIOB->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED8 | GPIO_OSPEEDR_OSPEED9);
  GPIOB->OSPEEDR |= (3U << GPIO_OSPEEDR_OSPEED8_Pos) | (3U << GPIO_OSPEEDR_OSPEED9_Pos);
  GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD8 | GPIO_PUPDR_PUPD9);
  GPIOB->AFR[1] &= ~(GPIO_AFRH_AFSEL8 | GPIO_AFRH_AFSEL9);
  GPIOB->AFR[1] |= (4U << GPIO_AFRH_AFSEL8_Pos) | (4U << GPIO_AFRH_AFSEL9_Pos);
}

void i2c_init(void) {
  I2C1->CR1 &= ~I2C_CR1_PE;
  I2C1->CR1 |= I2C_CR1_SWRST;
  I2C1->CR1 &= ~I2C_CR1_SWRST;

  I2C1->CR2 = 42U;
  I2C1->CCR = 210U;

  I2C1->TRISE = 43U;

  I2C1->CR1 |= I2C_CR1_PE;
}

uint8_t I2C_Master_Start(void) {
  uint32_t timeout = I2C_TIMEOUT;

  I2C1->CR1 |= I2C_CR1_START;

  while (!(I2C1->SR1 & I2C_SR1_SB)) {
    if (--timeout == 0) {
      return 0;
    }
  }

  (void)I2C1->SR1;

  return 1;
}

uint8_t I2C_Master_Send_Address(uint8_t address, uint8_t read) {
  uint32_t timeout = I2C_TIMEOUT;

  I2C1->DR = (uint8_t)((address << 1) | (read & 0x01U));

  while (!(I2C1->SR1 & I2C_SR1_ADDR)) {
    if (--timeout == 0) {
      return 0;
    }
  }

  (void)I2C1->SR1;
  (void)I2C1->SR2;

  return 1;
}

uint8_t I2C_Master_Send_Data(uint8_t data) {
  uint32_t timeout = I2C_TIMEOUT;

  I2C1->DR = data;

  while (!(I2C1->SR1 & I2C_SR1_TXE)) {
    if (--timeout == 0) {
      return 0;
    }
  }

  return 1;
}

void I2C_Master_Stop(void) {
  I2C1->CR1 |= I2C_CR1_STOP;
}

uint8_t I2C_Master_Write(uint8_t address, uint8_t *data, uint8_t length) {
  uint32_t timeout = I2C_TIMEOUT;

  if (length == 0) {
    return 0;
  }

  if (!I2C_Master_Start()) {
    return 0;
  }

  if (!I2C_Master_Send_Address(address, 0)) {
    I2C_Master_Stop();
    return 0;
  }

  for (uint8_t i = 0; i < length; i++) {
    if (!I2C_Master_Send_Data(data[i])) {
      I2C_Master_Stop();
      return 0;
    }
  }

  timeout = I2C_TIMEOUT;

  while (!(I2C1->SR1 & I2C_SR1_BTF)) {
    if (--timeout == 0) {
      I2C_Master_Stop();
      return 0;
    }
  }

  I2C_Master_Stop();

  return 1;
}

uint8_t I2C_Master_Read(uint8_t address, uint8_t *buffer, uint8_t length) {
  uint32_t timeout;

  if (length == 0) {
    return 0;
  }

  I2C1->CR1 |= I2C_CR1_ACK;

  if (!I2C_Master_Start()) {
    return 0;
  }

  if (!I2C_Master_Send_Address(address, 1)) {
    I2C_Master_Stop();
    return 0;
  }

  for (uint8_t i = 0; i < length; i++) {
    timeout = I2C_TIMEOUT;

    if (i == (length - 1)) {
      I2C1->CR1 &= ~I2C_CR1_ACK;
    }

    while (!(I2C1->SR1 & I2C_SR1_RXNE)) {
      if (--timeout == 0) {
        I2C1->CR1 &= ~I2C_CR1_ACK;
        I2C_Master_Stop();
        return 0;
      }
    }

    buffer[i] = (uint8_t)I2C1->DR;
  }

  I2C1->CR1 |= I2C_CR1_ACK;

  I2C_Master_Stop();

  return 1;
}