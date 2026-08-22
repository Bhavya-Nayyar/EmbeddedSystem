# STM32 UART Driver

A register-level UART driver for the **STM32F446RE**, written in C and developed using the STM32F446RE reference manual, datasheet, CMSIS device definitions, and STM32 startup/system files.

## Features

* Register-level UART configuration
* GPIO alternate-function configuration for UART
* Peripheral clock configuration
* Baud-rate configuration through the UART baud-rate register
* UART transmission and reception
* Interrupt-driven UART reception
* Receive ring buffer for asynchronous data handling
* Non-blocking reception through the ring buffer
* UART interrupt handling and status/error handling
* STM32 CMSIS register definitions
* CMake-based build system

## Hardware

* **Board:** STM32 Nucleo-F446RE
* **Debugger/Programmer:** ST-LINK

## Architecture

The UART peripheral is configured directly through its memory-mapped registers rather than through the STM32 HAL UART API.

The main driver is divided into two parts:

```text
UART peripheral
      │
      ├── uart.c
      │     └── UART configuration, TX/RX and interrupt handling
      │
      └── uart_ring_buffer.c
            └── Buffered reception
```

Incoming data is handled by the UART interrupt and placed into a ring buffer. The application can then consume the buffered data independently of the interrupt handler.

## Project Structure

```text
UART_Driver/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── uart.h
│   │   └── uart_ring_buffer.h
│   └── Src/
│       ├── main.c
│       ├── uart.c
│       ├── uart_ring_buffer.c
├── Drivers/
├── CMakeLists.txt
├── CMakePresets.json
├── STM32F446xx_FLASH.ld
├── startup_stm32f446xx.s
└── UART_Driver.ioc
```

## Build

The project uses **CMake** with an ARM GCC toolchain.

```bash
cmake --preset Debug
cmake --build --preset Debug
```

Available presets can be checked with:

```bash
cmake --list-presets
```

## Dependencies

* ARM GNU Toolchain
* CMake
* STM32F4 CMSIS/device headers
* STM32F4 startup and system files
* STM32F446RE hardware with ST-LINK
