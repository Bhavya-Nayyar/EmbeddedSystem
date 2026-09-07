# Embedded Systems

A collection of embedded systems projects developed using
microcontrollers, sensors, communication interfaces, RTOS concepts,
and firmware development tools.

## Projects

### 1. Smart Sensor Hub

An ESP32-based sensor monitoring system that integrates
multiple sensors and peripherals, with Wi-Fi communication, OLED
display, data logging, and power management.

**Technologies:** ESP32 · ESP-IDF · FreeRTOS · Wi-Fi · I2C · SPI

---

### 2. Firmware Over-The-Air (FOTA)

An ESP32-based FOTA system that downloads firmware over HTTP and updates the device using dual OTA partitions.

**Technologies:** ESP32 · ESP-IDF · HTTP · OTA · CMake

---

### 3. Fight the Timer

A bare-metal embedded game built on the ATmega328P that
combines timer-based gameplay with physical inputs, sensors, LEDs, a
7-segment display, and a buzzer.

The game demonstrates low-level AVR firmware development,
GPIO control, timer-based timing, button and tilt-sensor input,
LDR-based interaction, shift-register-driven display control, and
hardware feedback.

**Technologies:** ATmega328P · AVR-GCC · Embedded C · GPIO · Timers · 7-Segment Display · Shift Register · LDR · Tilt Sensor · Buzzer

---

### 4. STM32 UART Driver

A register-level UART driver developed for the
STM32F446RE. The driver configures the UART peripheral and its GPIO
interface directly through STM32 registers and implements
interrupt-driven reception with a ring buffer.

The project demonstrates low-level STM32 firmware
development, peripheral configuration, interrupt handling, UART
communication, and buffered data reception without relying on the STM32
HAL UART driver.

**Technologies:** STM32F446RE · Embedded C · CMSIS · Register-Level Programming · UART · Interrupts · Ring Buffer · CMake

---

### 5. STM32 SPI Driver

A register-level SPI driver developed for the STM32F446RE. The driver configures SPI1, GPIO, alternate functions, and chip select directly through CMSIS register definitions without using the STM32 HAL SPI API.

The project demonstrates SPI peripheral configuration, GPIO alternate-function mapping, polling-based data transmission, and software-controlled chip select.

**Technologies:** STM32F446RE · Embedded C · CMSIS · Register-Level Programming · SPI · GPIO · CMake

---

### 6. STM32 I2C Driver

A register-level I2C master driver developed for the STM32F446RE. The driver configures I2C1, GPIO, alternate functions, and I2C timing directly through CMSIS register definitions without using the STM32 HAL I2C API.

The project demonstrates I2C peripheral configuration, master transmit and receive operations, 7-bit addressing, START/STOP conditions, ACK/NACK handling, and polling-based communication.

**Technologies:** STM32F446RE · Embedded C · CMSIS · Register-Level Programming · I2C · GPIO · CMake

---

### 7. STM32–ESP32 CAN Bus Communication

A two-node CAN bus demo connecting an STM32F446RE and an ESP32 over a physical CAN bus at 250 kbps. The STM32 side uses HAL-based bxCAN with polling reception, while the ESP32 side uses the newer esp_twai driver with interrupt-driven reception and automatic bus-off recovery.

The project demonstrates CAN peripheral configuration, bit-timing calculation, standard-ID filtering, and cross-vendor CAN interoperability between two different CAN controller implementations.

**Technologies:** STM32F446RE · ESP32 · Embedded C · HAL · ESP-IDF · CAN/TWAI · CMake

## Repository Structure

```text
EmbeddedSystem/
│
├── Smart Sensor Hub/
│
├── Firmware_Over_The_Air/
│
├── Fight_The_Timer_Game/
│
├── UART_Driver/
│
├── SPI_Driver/
│
├── I2C_Driver/
│
└── CAN_Bus_/
