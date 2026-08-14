# Embedded Systems

A collection of embedded systems projects developed using microcontrollers, sensors, communication interfaces, RTOS concepts, and firmware development tools.

## Projects

### 1. Smart Sensor Hub

An ESP32-based sensor monitoring system that integrates multiple sensors and peripherals, with Wi-Fi communication, OLED display, data logging, and power management.

**Technologies:** ESP32 · ESP-IDF · FreeRTOS · Wi-Fi · I2C · SPI

[View Smart Sensor Hub](https://github.com/Bhavya-Nayyar/EmbeddedSystem/blob/main/Smart%20Sensor%20Hub)

---

### 2. Firmware Over-The-Air (FOTA)

An ESP32-based FOTA system that downloads firmware over HTTP and updates the device using dual OTA partitions.

**Technologies:** ESP32 · ESP-IDF · HTTP · OTA · CMake

[View FOTA Project](https://github.com/Bhavya-Nayyar/EmbeddedSystem/blob/main/Firmware_Over_The_Air)

---

### 3. Fight the Timer

A bare-metal embedded game built on the ATmega328P that combines timer-based gameplay with physical inputs, sensors, LEDs, a 7-segment display, and a buzzer.

The game demonstrates low-level AVR firmware development, GPIO control, timer-based timing, button and tilt-sensor input, LDR-based interaction, shift-register-driven display control, and hardware feedback.

**Technologies:** ATmega328P · AVR-GCC · Embedded C · GPIO · Timers · 7-Segment Display · Shift Register · LDR · Tilt Sensor · Buzzer

[View Fight the Timer](https://github.com/Bhavya-Nayyar/EmbeddedSystem/tree/main/Fight%20The%20Timer)

---

## Repository Structure

```text
EmbeddedSystem/
│
├── Smart Sensor Hub/
│
├── Firmware_Over_The_Air/
│
└── Fight The Timer/
```

## Tools & Technologies

* C / Embedded C
* ATmega328P / AVR
* ESP32
* ESP-IDF
* FreeRTOS
* GPIO
* Timers
* ADC
* Wi-Fi
* I2C / SPI / UART
* Sensors & Peripherals
* Git & GitHub
* CMake / Ninja
* AVR-GCC
