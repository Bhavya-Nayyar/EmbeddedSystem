# STM32 ↔ ESP32 CAN Bus Communication

Two-node CAN demo using native peripherals on both sides — STM32's **bxCAN** via HAL, and ESP32's **TWAI** via the `esp_twai` driver. Both nodes exchange periodic frames over a physical CAN bus at 250 kbps.

## Hardware

- STM32 Nucleo-F446RE (CAN1)
- ESP32 (TWAI)
- 2× SN65HVD230 CAN transceivers (3.3V, one per node)
- 2× 120 Ω termination resistors (one at each end of the bus)
- Common ground between both boards

### Wiring

| Signal | STM32 | ESP32 |
|---|---|---|
| CAN RX | `PB8` (remapped CAN1) | GPIO `21` |
| CAN TX | `PB9` (remapped CAN1) | GPIO `22` |

> `PB8`/`PB9` also map to `D15`/`D14` (I2C1 SCL/SDA) on the Nucleo Arduino header — avoid combining with I2C1.

Both transceivers' `CANH`/`CANL` share one bus, terminated 120 Ω at each end.

STM32 debug logs go out over `USART2` at 115200 baud (ST-LINK VCP).

## Bus Config

| Parameter | Value |
|---|---|
| Bitrate | 250 kbps |
| STM32 timing | Prescaler 12, TimeSeg1 11 Tq, TimeSeg2 2 Tq (14 Tq/bit @ 42 MHz APB1) |
| Frame format | Standard ID, data frame, DLC 8 |
| Auto-retransmission | Disabled (STM32) |

## Message IDs

| Direction | ID | Payload |
|---|---|---|
| STM32 → ESP32 | `0x101` | `[counter, 0xAA..0x22]` |
| ESP32 → STM32 | `0x100` | `[counter, 0x02..0x08]` |

Each node transmits once per second.

## Firmware

**STM32** (`main.c`, HAL): polling-based RX in the main loop (`HAL_CAN_GetRxMessage`), 1 Hz TX, LED toggles on receipt.

**ESP32** (`esp_twai` API): interrupt-driven RX via `on_rx_done` callback, auto bus-off recovery via `on_state_change`, TX confirmed with `twai_node_transmit_wait_all_done`.

## Build & Flash

**STM32:** flash via ST-LINK, monitor at 115200-8-N-1.

**ESP32:**
```bash
idf.py set-target esp32
idf.py build
idf.py -p <PORT> flash monitor
```

## Known Issues

- STM32 RX filter (`0x0000`/`0x0000` mask) accepts every ID — not actually filtering to `0x100` as the logs imply.
- STM32 has no bus-off recovery (ESP32 does).
- RX is polled on STM32, interrupt-driven on ESP32 — asymmetric by choice, not yet unified.

## Future Improvements

- Fix STM32 filter to match only `0x100`.
- Add bus-off detection + recovery on the STM32 side.
- Move STM32 RX to interrupt-driven (`HAL_CAN_RxFifo0MsgPendingCallback`) for symmetry with ESP32.
- Add a sequence-counter check on both sides to detect dropped frames.
- Extend to a 3-node bus to test arbitration under real contention.
