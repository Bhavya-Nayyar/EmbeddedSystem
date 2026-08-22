#include "uart.h"
#include "stm32f446xx.h"
#include "uart_ring_buffer.h"
#include <stdbool.h>
#include <stdint.h>

#define UART_BUFFER_SIZE 16

static uint8_t buffer[UART_BUFFER_SIZE];

static struct ring_buffer tx_buffer = {
    .buffer = buffer,
    .size = sizeof(buffer),
};

static void clock_init(void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
}

static void config_pinMode(void) {
  GPIOA->MODER &= ~((3U << 4) | (3U << 6));

  GPIOA->MODER |= ((2U << 4) | (2U << 6));
}

static void set_AF(void) {
  GPIOA->AFR[0] &= ~((15U << 8) | (15U << 12));

  GPIOA->AFR[0] |= ((7U << 8) | (7U << 12));
}

static void config_usart2_pins(void) {
  config_pinMode();
  set_AF();
}

static void config_usart2(void) {
  USART2->CR1 &= ~USART_CR1_UE;

  USART2->CR1 = 0;

  USART2->CR2 = 0;

  USART2->CR3 = 0;

  USART2->BRR = 0x016D;

  USART2->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

static void uart_tx_disable_interrupt(void) { USART2->CR1 &= ~USART_CR1_TXEIE; }

static void uart_tx_start(void) {
  if (!ring_buffer_is_empty(&tx_buffer)) {
    USART2->DR = ring_buffer_get(&tx_buffer);

    USART2->CR1 |= USART_CR1_TXEIE;
  }
}

void uart_ISR(void) {
  if (USART2->SR & USART_SR_TXE) {
    if (!ring_buffer_is_empty(&tx_buffer)) {
      USART2->DR = ring_buffer_get(&tx_buffer);
    } else {
      uart_tx_disable_interrupt();
    }
  }
}

void uart_putchar_interrupt(char c) {
  while (ring_buffer_is_full(&tx_buffer)) {
  }

  uart_tx_disable_interrupt();

  bool tx_ongoing = !ring_buffer_is_empty(&tx_buffer);

  ring_buffer_put(&tx_buffer, (uint8_t)c);

  if (!tx_ongoing) {
    uart_tx_start();
  }
}

void transmit_uart(uint8_t data) {  
  while (!(USART2->SR & USART_SR_TXE)) {
  }
   
  USART2->DR = data;
}

uint8_t receive_uart(void) {
  if (USART2->SR & USART_SR_RXNE) {
    return (uint8_t)USART2->DR;
  }

  return 0;
}

void uart_init(void) {
  clock_init();

  config_usart2_pins();

  config_usart2();

  NVIC_EnableIRQ(USART2_IRQn);
}