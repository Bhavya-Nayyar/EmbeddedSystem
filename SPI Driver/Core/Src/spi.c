#include "spi.h"
#include "stm32f446xx.h"

/*
PA5    SPI1_SCK
PA6    SPI1_MISO
PA7    SPI1_MOSI
PB6    GPIO Output -> CS
*/

void clock_init(void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
}

void gpio_init(void) {
  GPIOB->MODER &= ~(3U << (6U * 2U));
  GPIOB->MODER |= (1U << (6U * 2U));
  GPIOB->OTYPER &= ~(1U << 6U);
  GPIOB->OSPEEDR &= ~(3U << (6U * 2U));
  GPIOB->OSPEEDR |= (2U << (6U * 2U));
  GPIOB->PUPDR &= ~(3U << (6U * 2U));

  GPIOA->MODER &= ~(3U << (5U * 2U));
  GPIOA->MODER |= (2U << (5U * 2U));
  GPIOA->OTYPER &= ~(1U << 5U);
  GPIOA->OSPEEDR &= ~(3U << (5U * 2U));
  GPIOA->OSPEEDR |= (2U << (5U * 2U));
  GPIOA->PUPDR &= ~(3U << (5U * 2U));

  GPIOA->MODER &= ~(3U << (6U * 2U));
  GPIOA->MODER |= (2U << (6U * 2U));
  GPIOA->OTYPER &= ~(1U << 6U);
  GPIOA->OSPEEDR &= ~(3U << (6U * 2U));
  GPIOA->OSPEEDR |= (2U << (6U * 2U));
  GPIOA->PUPDR &= ~(3U << (6U * 2U));

  GPIOA->MODER &= ~(3U << (7U * 2U));
  GPIOA->MODER |= (2U << (7U * 2U));
  GPIOA->OTYPER &= ~(1U << 7U);
  GPIOA->OSPEEDR &= ~(3U << (7U * 2U));
  GPIOA->OSPEEDR |= (2U << (7U * 2U));
  GPIOA->PUPDR &= ~(3U << (7U * 2U));
}

void alt_func(void) {
  GPIOA->AFR[0] &= ~(0xFU << (5U * 4U));
  GPIOA->AFR[0] |= (5U << (5U * 4U));

  GPIOA->AFR[0] &= ~(0xFU << (6U * 4U));
  GPIOA->AFR[0] |= (5U << (6U * 4U));

  GPIOA->AFR[0] &= ~(0xFU << (7U * 4U));
  GPIOA->AFR[0] |= (5U << (7U * 4U));
}

void spi_init(void) {
  SPI1->CR1 &= ~SPI_CR1_SPE;

  SPI1->CR1 |= SPI_CR1_MSTR | SPI_CR1_BR_2 | SPI_CR1_BR_0 | SPI_CR1_SSM | SPI_CR1_SSI;

  SPI1->CR1 &= ~((SPI_CR1_DFF) | (SPI_CR1_LSBFIRST) | (SPI_CR1_CPOL | SPI_CR1_CPHA));
}

void spi_enable(void) {
  SPI1->CR1 |= SPI_CR1_SPE;
}

void spi_tx_8bit(uint8_t data) {
  while (!(SPI1->SR & SPI_SR_TXE));

  *((volatile uint8_t *)&SPI1->DR) = data;
}