#ifndef UART_H
#define UART_H

#include "stdint.h"

void uart_ISR(void);

void uart_putchar_interrupt(char c);

uint8_t receive_uart(void);

void transmit_uart(uint8_t data);

void uart_init(void);

#endif