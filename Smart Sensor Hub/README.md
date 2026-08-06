# Smart Sensor Hub (ESP32 + ESP-IDF)

## Overview

Smart Sensor Hub is an embedded IoT system built using the ESP32 and the ESP-IDF framework. The project demonstrates real-time acquisition of environmental data from multiple sensors, local visualization on LCD/OLED displays, SD card logging, and wireless transmission of sensor data to a web dashboard over Wi-Fi.

The project emphasizes modular firmware architecture, FreeRTOS-based multitasking, peripheral interfacing, and network communication.

---

## Features

* Real-time temperature measurement using a thermistor
* Ambient temperature and humidity monitoring using DHT11
* Motion detection using a PIR sensor
* Sound detection using an electret microphone module
* OLED display for environmental data
* Character LCDs for motion and sound status
* SD card data logging
* Wi-Fi connectivity
* HTTP client for transmitting sensor data
* Live web dashboard displaying sensor readings
* Modular ESP-IDF component architecture
* FreeRTOS multitasking with synchronization

---

## Hardware Used

| Component                           | Quantity    |
| ----------------------------------- | ----------- |
| ESP32 Development Board             | 1           |
| DHT11 Temperature & Humidity Sensor | 1           |
| 10K NTC Thermistor                  | 1           |
| PIR Motion Sensor (HC-SR501)        | 1           |
| Electret Microphone Sound Sensor    | 1           |
| SSD1306 OLED Display (I2C)          | 1           |
| 16x2 LCD with I2C Backpack          | 2           |
| Micro SD Card Module (SPI)          | 1           |
| Breadboard                          | 1           |
| Jumper Wires                        | As required |
| 10KΩ Resistor                       | 1           |

---

## Software

* ESP-IDF v6.x
* FreeRTOS
* Visual Studio Code
* Espressif IDF Extension
* CMake
* Ninja
* HTML
* CSS
* JavaScript

---

## Project Structure

```
Smart-Sensor-Hub/
│
├── main/
│   └── main.c
│
├── components/
│   ├── DHT11_Sensor/
│   ├── Thermistor/
│   ├── PIR_Sensor/
│   ├── Sound_Sensor/
│   ├── OLED_Display/
│   ├── PIR_LCD/
│   ├── Sound_LCD/
│   ├── SD_Card/
│   ├── WiFi/
│   ├── HTTP_Client/
│   ├── Sensor_Data/
│   └── ...
│
├── Server/
│   ├── server.js
│   ├── package.json
│   ├── public/
│   │   ├── index.html
│   │   ├── style.css
│   │   └── script.js
│   └── ...
│
├── README.md
```

---

## System Architecture

```
                 +-------------------+
                 |      ESP32        |
                 +-------------------+
                          |
      -------------------------------------------------
      |        |         |          |         |        |
      |        |         |          |         |        |
   DHT11   Thermistor    PIR      Sound     SD Card   Wi-Fi
      |        |          |          |          |        |
      ---------------- Sensor Data ---------------------
                          |
                   Shared Sensor Structure
                          |
             -------------------------------
             |              |              |
          OLED LCDs     SD Logging     HTTP Client
                                           |
                                           |
                                    Local Web Server
                                           |
                                           |
                                     Browser Dashboard
```

---

## Sensor Information

### DHT11

Measures:

* Temperature (°C)
* Relative Humidity (%)

---

### Thermistor

Measures:

* Ambient temperature using ADC conversion

---

### PIR Sensor

Detects:

* Human motion

Output:

* Motion Detected
* No Motion

---

### Sound Sensor

Detects:

* Loud sound events

Output:

* Sound Detected
* Quiet

---

## Displays

### OLED

Displays

* DHT11 Temperature
* DHT11 Humidity
* Thermistor Temperature

### LCD 1

Displays

* PIR Status

### LCD 2

Displays

* Sound Status

---

## Wi-Fi Dashboard

The ESP32 periodically sends sensor data over HTTP to a local Node.js server.

Example JSON payload:

```json
{
  "temperature": 30.5,
  "humidity": 67.2,
  "thermistor": 29.8,
  "pir_motion": true,
  "sound_detected": false
}
```

The server hosts a web page that updates automatically to display the latest sensor readings.

---

## FreeRTOS Tasks

Typical firmware tasks include:

* DHT11 Task
* Thermistor Task
* PIR Task
* Sound Sensor Task
* OLED Display Task
* PIR LCD Task
* Sound LCD Task
* SD Card Logger Task
* Wi-Fi Communication Task

Shared sensor data is protected using a mutex to ensure thread-safe access across tasks.

---

## Build Instructions

### ESP32 Firmware

Clone the repository:

```bash
git clone https://github.com/<your-username>/Smart-Sensor-Hub.git
```

Navigate to the project:

```bash
cd Smart-Sensor-Hub
```

Configure the project:

```bash
idf.py menuconfig
```

Build:

```bash
idf.py build
```

Flash:

```bash
idf.py -p COMx flash
```

Monitor:

```bash
idf.py monitor
```

---

## Running the Web Dashboard

Navigate to the server directory:

```bash
cd Server
```

Install dependencies:

```bash
npm install
```

Start the server:

```bash
node server.js
```

Open your browser:

```
http://localhost:3000
```

To access the dashboard from another device on the same Wi-Fi network:

```
http://<YOUR_PC_IP_ADDRESS>:3000
```

---

## Future Improvements

* MQTT support
* OTA firmware updates
* ESP RainMaker integration
* Sensor history graphs
* Data storage using SQLite
* User authentication
* Mobile-responsive dashboard
* Cloud integration (AWS, Azure, or Firebase)
* Alarm notifications
* Device configuration through the web interface

---

## Learning Outcomes

This project demonstrates practical experience with:

* Embedded C
* ESP-IDF
* FreeRTOS
* GPIO
* ADC
* I2C
* SPI
* SD Card interfacing
* Wi-Fi networking
* HTTP communication
* JSON serialization
* Multitasking
* Mutex synchronization
* Modular firmware design
* Client-server communication
* Embedded IoT system development

---

Embedded Systems | Firmware Development | IoT
