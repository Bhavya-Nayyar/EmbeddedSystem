# Fight the Timer Game

A simple embedded reaction game built on the **Arduino Uno (ATmega328P)** using **bare-metal C**.

The player starts a timer by pressing a button. The timer counts from `0` toward `9` on a 7-segment display. The player must tilt the board before the timer reaches `9`. Every detected tilt resets the timer to `0`.

If the timer reaches `9`, the player loses. The display turns off, the buzzer sounds, and an RGB LED animation indicates that the game is over.

The project is written directly for the ATmega328P using **AVR-GCC**, without the Arduino framework.

## Game Flow

```text
                 ┌───────────────┐
                 │     IDLE      │
                 │ Display OFF   │
                 └───────┬───────┘
                         │
                    Press Button
                         │
                         ▼
                 ┌───────────────┐
                 │ TIMER STARTS  │
                 │   Display 0   │
                 └───────┬───────┘
                         │
                         ▼
                 ┌───────────────┐
                 │    COUNTING   │
                 │ 0 → 1 → ...   │
                 └───────┬───────┘
                         │
              ┌──────────┴──────────┐
              │                     │
        Tilt detected          Reaches 9
              │                     │
              ▼                     ▼
        Reset to 0            ┌─────────────┐
              │                │  GAME OVER  │
              │                └──────┬──────┘
              │                       │
              │                 Display OFF
              │                       │
              │                  Buzzer ON
              │                       │
              │                RGB Animation
              │                       │
              └───────► New Game ◄───┘
```

## Hardware

* Arduino Uno R3
* ATmega328P
* 7-segment display
* 8-bit shift register
* Push button
* Tilt sensor
* RGB LED
* Buzzer
* S8050 NPN transistor
* Resistors
* Breadboard
* Jumper wires

## Pin Configuration

| Function             | ATmega328P | Arduino |
| -------------------- | ---------: | ------: |
| Shift Register DATA  |        PD3 |      D3 |
| Shift Register CLOCK |        PD4 |      D4 |
| Shift Register LATCH |        PD2 |      D2 |
| Start Button         |        PD5 |      D5 |
| Tilt Sensor          |        PD6 |      D6 |
| RGB Red              |        PD7 |      D7 |
| RGB Green            |        PB0 |      D8 |
| RGB Blue             |        PB1 |      D9 |
| Buzzer               |        PB2 |     D10 |

The **button** and **tilt sensor** inputs use the ATmega328P's internal pull-up resistors.

## 7-Segment Display

The display is controlled through an 8-bit shift register using three GPIO pins:

```text
ATmega328P
    │
    ├── DATA
    ├── CLOCK
    └── LATCH
          │
          ▼
    Shift Register
          │
          ▼
    7-Segment Display
```

`shift_out()` sends the display pattern serially, while `display()` toggles the latch pin to update the shift-register outputs.

The digit patterns currently used are:

| Digit | Pattern |
| ----: | ------: |
|     0 |  `0xA0` |
|     1 |  `0xF9` |
|     2 |  `0xC4` |
|     3 |  `0xD0` |
|     4 |  `0x99` |
|     5 |  `0x92` |
|     6 |  `0x82` |
|     7 |  `0xF8` |
|     8 |  `0x00` |
|     9 |  `0x90` |

These values depend on the specific display and shift-register wiring.

## Game Logic

### 1. Idle

The system initially:

* Turns the RGB LED off.
* Turns the buzzer off.
* Turns the 7-segment display off.
* Waits for the start button.

### 2. Start

When the button is pressed:

```text
Display → 0
Timer   → starts
```

### 3. Counting

The timer progresses approximately once per second:

```text
0 → 1 → 2 → 3 → 4 → ... → 9
```

During the counting period, the tilt sensor is continuously polled.

### 4. Tilt

When a tilt is detected:

```text
Current digit → 0
```

The timer then starts counting upward again.

The player must keep tilting the board at the appropriate time to prevent the counter from reaching `9`.

### 5. Game Over

If the timer reaches `9` without a successful tilt:

```text
7-segment → OFF
Buzzer    → ON
RGB LED   → Animation
```

The buzzer sounds for approximately **1 second**.

The RGB LED then displays:

```text
RED
GREEN
BLUE
YELLOW
CYAN
MAGENTA
WHITE
```

After the animation finishes, the RGB LED and buzzer are switched off and the system returns to the idle state.

## Software

The project uses:

* C
* AVR-GCC
* AVR-LibC
* AVR Binutils
* avrdude
* GNU Make

The program directly manipulates ATmega328P registers.

## Project Structure

```text
FIGHT THE TIMER GAME/
│
├── .vscode/
│   └── c_cpp_properties.json
│
├── src/
│   └── main.c
│
├── .gitignore
├── Makefile
└── README.md
```

## Build System

The project uses a simple Makefile.

### Build

From the project root:

```bash
make
```

This performs:

```text
main.c
  ↓
avr-gcc
  ↓
main.elf
  ↓
avr-objcopy
  ↓
main.hex
```

The generated files are:

```text
src/main.elf
src/main.hex
```

### Upload

Connect the Arduino Uno and run:

```bash
make upload
```

The Makefile uses:

```text
Programmer: arduino
MCU:        atmega328p
Port:       COM6
Baud rate:  115200
```

If the Arduino is connected to a different COM port, change:

```make
PORT = COM6
```

in the Makefile.

### Clean

To remove generated build files:

```bash
make clean
```

This removes:

```text
src/main.elf
src/main.hex
```

## Current Implementation Limitations

The current implementation intentionally keeps the firmware simple, but there are several technical limitations.

### Software Timing

The timer is implemented using `_delay_ms()` rather than a hardware timer.

The approximate one-second interval is implemented as:

```text
100 × 10 ms ≈ 1 second
```

This is adequate for a simple game but is not a precision timing mechanism.

### Polling

The button and tilt sensor are polled by the main loop.

No external interrupts are currently used.

### Tilt Sensor

Mechanical tilt sensors can produce transient or bouncing signals. The current implementation uses delays and waits for the sensor to return to its inactive state.

### Blocking Code

The buzzer and RGB animation use blocking delays. During these animations, the main application does not perform other work.

## Possible Improvements

Future versions could improve the project by:

* Using **Timer1** for accurate game timing.
* Using **external interrupts** for tilt detection.
* Implementing a proper finite state machine.
* Improving tilt-sensor debouncing.
* Generating buzzer tones using hardware PWM.
* Adding difficulty levels.
* Randomizing the required reaction timing.
* Adding a score system.
* Adding a countdown/start animation.
* Separating the firmware into multiple modules.
* Adding unit-testable game logic.
* Adding configurable parameters to the Makefile.
* Adding automatic detection/configuration of the serial port.
