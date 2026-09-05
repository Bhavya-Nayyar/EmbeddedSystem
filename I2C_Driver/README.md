# I2C Register-Level Driver — STM32F446RE

A **register-level I²C master driver** for the STM32F446RE, implemented by directly configuring the STM32 RCC, GPIO, and I²C peripheral registers without using HAL I²C APIs.

## Features

* STM32F446RE / Nucleo-F446RE
* I2C1 Master
* PB8 — SCL
* PB9 — SDA
* 100 kHz Standard Mode
* 7-bit addressing
* Master transmit and receive
* START / STOP conditions
* ACK/NACK handling
* Basic timeout handling

## Project Structure

```text
I2C_Driver/
├── Core/
│   ├── Inc/
│   │   └── i2c.h
│   └── Src/
│       ├── i2c.c
│       └── main.c
├── Drivers/
├── CMakeLists.txt
└── README.md
```

## Hardware

| STM32F446RE | I²C |
| ----------- | --- |
| PB8         | SCL |
| PB9         | SDA |

I²C requires pull-up resistors on SDA and SCL. Typical value: **4.7 kΩ to 3.3 V**.

