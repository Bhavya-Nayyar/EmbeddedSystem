#ifndef SPI_H
#define SPI_H

#include <sys/types.h>
void clock_init(void);

void gpio_init(void);

void alt_func(void);

void spi_init(void);

void spi_enable(void);

void spi_tx_8bit(uint8_t data);

#endif