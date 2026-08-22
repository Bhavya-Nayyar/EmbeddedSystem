# Embedded Systems

A collection of embedded systems projects developed using microcontrollers, sensors, communication interfaces, RTOS concepts, and firmware development tools.

## Projects

### 1. Smart Sensor Hub

An ESP32-based sensor monitoring system that integrates multiple sensors and peripherals, with Wi-Fi communication, OLED display, data logging, and power management.

**Technologies:** ESP32 · ESP-IDF · FreeRTOS · Wi-Fi · I2C · SPI

---

### 2. Firmware Over-The-Air (FOTA)

An ESP32-based FOTA system that downloads firmware over HTTP and updates the device using dual OTA partitions.

**Technologies:** ESP32 · ESP-IDF · HTTP · OTA · CMake

---

### 3. Fight the Timer

A bare-metal embedded game built on the ATmega328P that combines timer-based gameplay with physical inputs, sensors, LEDs, a 7-segment display, and a buzzer.

The game demonstrates low-level AVR firmware development, GPIO control, timer-based timing, button and tilt-sensor input, LDR-based interaction, shift-register-driven display control, and hardware feedback.

**Technologies:** ATmega328P · AVR-GCC · Embedded C · GPIO · Timers · 7-Segment Display · Shift Register · LDR · Tilt Sensor · Buzzer

---

### 4. STM32 UART Driver

A register-level UART driver developed for the STM32F446RE. The driver configures the UART peripheral and its GPIO interface directly through STM32 registers and implements interrupt-driven reception with a ring buffer.

The project demonstrates low-level STM32 firmware development, peripheral configuration, interrupt handling, UART communication, and buffered data reception without relying on the STM32 HAL UART driver.

**Technologies:** STM32F446RE · Embedded C · CMSIS · Register-Level Programming · UART · Interrupts · Ring Buffer · CMake

---

## Repository Structure

```text
EmbeddedSystem/
│
├── Smart Sensor Hub/
│
├── Firmware_Over_The_Air/
│
├── Fight The Timer/
│
└── UART_Driver/
```

## Tools & Technologies

* C / Embedded C
* ATmega328P / AVR
* STM32F446RE
* ESP32
* CMSIS
* ESP-IDF
* FreeRTOS
* GPIO
* Timers
* ADC
* UART
* I2C / SPI
* Interrupts
* Ring Buffers
* Wi-Fi
* OTA
* Sensors & Peripherals
* Git & GitHub
* CMake / Ninja
* AVR-GCC
* ARM GNU Toolchain
