# STM32 SPI Driver

A **register-level SPI driver for the STM32F446RE**, developed using CMSIS device definitions and the STM32F4 reference manual. The project focuses on understanding SPI peripheral configuration and communication without using the STM32 HAL SPI APIs.

## Overview

The driver directly configures:

* RCC peripheral clocks
* GPIO modes and alternate functions
* SPI1 control registers
* SPI status registers
* SPI data register
* Software-controlled chip select (CS)

The goal is to understand how SPI works at the peripheral-register level rather than simply using a high-level library.

## Hardware

* **Board:** STM32 Nucleo-F446RE

## Pin Configuration

| STM32 Pin | Function         | Nucleo Arduino Pin |
| --------- | ---------------- | ------------------ |
| PA5       | SPI1_SCK         | D13                |
| PA6       | SPI1_MISO        | D12                |
| PA7       | SPI1_MOSI        | D11                |
| PB6       | Chip Select (CS) | D10                |

`PB6` is configured as a normal GPIO output and is controlled manually by the application.

## Driver Structure

```text
SPI_Driver/
├── Core/
│   ├── Inc/
│   │   └── spi.h
│   └── Src/
│       ├── main.c
│       └── spi.c
├── Drivers/
│   ├── CMSIS/
│   └── STM32F4xx_HAL_Driver/
├── cmake/
├── CMakeLists.txt
└── README.md
```

## Future Improvements

Possible extensions include:

1. Implement `spi_rx_8bit()`
2. Implement full-duplex `spi_transfer()`
3. Add configurable CPOL/CPHA
4. Add configurable SPI clock prescalers
5. Add timeout handling
6. Add interrupt-driven transfers
7. Add DMA-based transfers
8. Support multiple chip-select lines
9. Connect the driver to an actual SPI sensor/display
10. Compare register-level implementation with STM32 LL and HAL implementations
