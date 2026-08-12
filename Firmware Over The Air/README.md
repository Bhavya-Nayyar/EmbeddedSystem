# ESP32 Firmware Over-The-Air (FOTA)

A Firmware Over-The-Air (FOTA) update system for the ESP32 using **ESP-IDF**, **HTTP**, and the ESP-IDF OTA APIs.

The project demonstrates how an ESP32 can download a new firmware image from an HTTP server, write it to the inactive OTA partition, validate the image, switch the boot partition, and reboot into the updated firmware.

---

## Features

* HTTP-based firmware download
* Dual OTA application partitions (`ota_0` and `ota_1`)
* Automatic selection of the inactive OTA partition
* Firmware validation using ESP-IDF OTA APIs
* Boot partition switching after a successful update
* Automatic reboot into the new firmware
* LED indication of OTA status
* OLED display for system/OTA information
* Built using ESP-IDF rather than the Arduino framework

---

## Project Architecture

```text
                    ┌──────────────────────┐
                    │      HTTP Server     │
                    │                      │
                    │   New Firmware.bin   │
                    └──────────┬───────────┘
                               │
                            HTTP GET
                               │
                               ▼
                    ┌──────────────────────┐
                    │        ESP32         │
                    │                      │
                    │     HTTP Client      │
                    │          │           │
                    │          ▼           │
                    │      OTA API         │
                    │          │           │
                    │          ▼           │
                    │   Inactive OTA Slot  │
                    └──────────┬───────────┘
                               │
                            Reboot
                               │
                               ▼
                    ┌──────────────────────┐
                    │    New Firmware      │
                    │      Executes        │
                    └──────────────────────┘
```

---

## How FOTA Works

The ESP32 uses two OTA application partitions.

For example:

```text
Current firmware → ota_0
New firmware     → ota_1
```

The update process is:

1. ESP32 connects to the network.
2. ESP32 starts an HTTP connection to the firmware server.
3. The firmware image is downloaded in chunks.
4. The downloaded data is written to the inactive OTA partition.
5. The OTA image is finalized and validated.
6. ESP32 sets the newly written partition as the next boot partition.
7. ESP32 reboots.
8. The bootloader starts the new firmware.

After the reboot:

```text
ota_1 → Running firmware
ota_0 → Available for the next update
```

The process can therefore continue in both directions.

---

## Partition Layout

The project uses an OTA partition configuration similar to:

```text
+-----------------------------+
| Partition Table             |
+-----------------------------+
| NVS                         |
+-----------------------------+
| PHY Init                    |
+-----------------------------+
| OTA 0                       |
|                             |
| Application Firmware        |
+-----------------------------+
| OTA 1                       |
|                             |
| Application Firmware        |
+-----------------------------+
```

The two application slots are required because the ESP32 needs a separate partition in which to download the new firmware while continuing to run the currently active firmware.

---

## Technologies Used

| Component               | Technology        |
| ----------------------- | ----------------- |
| Microcontroller         | ESP32             |
| Framework               | ESP-IDF v6.0.1    |
| Language                | C                 |
| OTA                     | ESP-IDF OTA API   |
| Network                 | Wi-Fi             |
| Transfer protocol       | HTTP              |
| Firmware format         | `.bin`            |
| Build system            | CMake + Ninja     |
| Display                 | SSD1306 OLED      |
| Development environment | VS Code / ESP-IDF |

---

## Project Structure

```text
Firmware_Over_The_Air/
│
├── main/
│   ├── ...
│   └── CMakeLists.txt
│
├── components/
│   ├── FOTA/
│   │   ├── ...
│   │   └── CMakeLists.txt
│   │
│   ├── OLED/
│   │   ├── ...
│   │   └── CMakeLists.txt
│   │
│   └── LED/
│       ├── ...
│       └── CMakeLists.txt
│
├── partitions.csv
├── CMakeLists.txt
├── sdkconfig
└── README.md
```

---

## Requirements

### Hardware

* ESP32 development board
* USB cable
* SSD1306 OLED display (if using the display functionality)
* LED and resistor (if using the LED status indicator)
* Wi-Fi network

### Software

* ESP-IDF v6.0.1
* Python environment provided by ESP-IDF
* CMake
* Ninja
* Git
* An HTTP server capable of serving the firmware `.bin` file

---

## Building the Project

Open an ESP-IDF terminal and navigate to the project directory.

Build the project:

```bash
idf.py build
```

If the build succeeds, ESP-IDF generates the application binary inside the `build` directory.

---

## Flashing the ESP32

Connect the ESP32 and identify its serial port.

Flash the project:

```bash
idf.py -p COMx flash
```

Replace `COMx` with the actual ESP32 serial port.

For example:

```bash
idf.py -p COM4 flash
```

Monitor the device:

```bash
idf.py -p COM4 monitor
```

Or build, flash, and monitor together:

```bash
idf.py -p COM4 flash monitor
```

---

## Firmware Server

The OTA firmware must be available from an HTTP server.

The server should provide the compiled firmware binary through an HTTP URL, for example:

```text
http://<server-ip>:<port>/firmware.bin
```

The ESP32 HTTP client requests this file during the OTA process.

The firmware URL configured in the project must therefore point to a server that is reachable from the ESP32 over the network.

---

## Performing an OTA Update

### 1. Build the new firmware

Modify the application, then build:

```bash
idf.py build
```

This generates a new application binary.

### 2. Place the firmware on the HTTP server

Upload/copy the new `.bin` file to the location served by the HTTP server.

### 3. Start the server

Make sure the server is running and accessible from the ESP32.

### 4. Start the OTA process

The ESP32 connects to the server and downloads the new firmware.

Conceptually:

```text
ESP32
  │
  │ HTTP GET
  ▼
HTTP Server
  │
  │ firmware.bin
  ▼
ESP32
  │
  ▼
Inactive OTA partition
  │
  ▼
Validate firmware
  │
  ▼
Set boot partition
  │
  ▼
Reboot
  │
  ▼
New firmware
```

---

## OTA Safety

The current firmware is not overwritten during the download.

Instead, the new firmware is written to the inactive OTA partition.

For example:

```text
Before update:

ota_0 → Running firmware
ota_1 → New firmware target


After successful update:

ota_0 → Previous firmware
ota_1 → Running firmware
```

This is the fundamental mechanism that makes ESP-IDF's dual-partition OTA approach possible.

---

## Important ESP-IDF APIs

The project is based on the following ESP-IDF concepts/APIs:

* `esp_http_client`
* `esp_ota_begin()`
* `esp_ota_write()`
* `esp_ota_end()`
* `esp_ota_set_boot_partition()`
* `esp_ota_get_boot_partition()`
* `esp_ota_get_running_partition()`

The HTTP client handles the firmware download, while the OTA APIs handle writing and managing the firmware image.

---

## OTA Update Flow

```text
Start
  │
  ▼
Connect to Wi-Fi
  │
  ▼
Determine OTA partition
  │
  ▼
Start HTTP connection
  │
  ▼
Download firmware
  │
  ▼
Write firmware to inactive partition
  │
  ▼
Download complete?
  │
  ├── No ──► Continue downloading
  │
  └── Yes
        │
        ▼
    Validate image
        │
        ├── Failed ──► Abort OTA
        │
        └── Success
              │
              ▼
       Set boot partition
              │
              ▼
            Reboot
              │
              ▼
       New firmware runs
```

---

## Error Handling

An OTA update should not switch the boot partition if the firmware download or validation fails.

Typical failure points include:

* Wi-Fi connection failure
* HTTP connection failure
* Server unavailable
* Incorrect firmware URL
* Incomplete firmware download
* Invalid firmware image
* OTA write failure
* Firmware validation failure

In these cases, the currently running firmware remains the active firmware.

---

## Development Notes

This project is primarily intended to demonstrate the fundamentals of ESP32 FOTA:

1. ESP32 partitioning for OTA
2. HTTP firmware delivery
3. Streaming firmware data
4. Writing firmware to an inactive partition
5. Firmware validation
6. Boot partition management
7. Rebooting into the updated application

Production OTA systems should additionally consider HTTPS/TLS, firmware authentication, firmware signing, version checking, rollback handling, secure boot, and flash encryption.

---

## ESP-IDF Documentation

* ESP-IDF OTA API:
  https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/ota.html

* ESP-IDF HTTP Client:
  https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/protocols/esp_http_client.html

* ESP-IDF OTA examples:
  https://github.com/espressif/esp-idf/tree/master/examples/system/ota

---

## Future Improvements

* HTTPS instead of HTTP
* Firmware version checking
* Firmware authentication/signing
* Secure Boot
* Flash Encryption
* Automatic rollback after failed boot
* OTA update progress indication
* Remote firmware version checking
* More robust update failure recovery

---
