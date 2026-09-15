/*
 * ESP32 side — nRF24L01 SENDER (test link with STM32 receiver)
 * Based on nopnop2002/esp-idf-mirf
 *
 * Wiring (from your screenshot):
 *   VCC  -> 3.3V  (DO NOT use 5V unless your adapter board has a confirmed
 *                  onboard 3.3V regulator — verify this before powering up)
 *   GND  -> GND
 *   CE   -> GPIO 4
 *   CSN  -> GPIO 5   (VSPI CS)
 *   SCK  -> GPIO 18  (VSPI CLK)
 *   MISO -> GPIO 19
 *   MOSI -> GPIO 23
 *
 * IMPORTANT: set these to match the STM32 side EXACTLY:
 *   - Address:      "FGHIJ"   (5 bytes, ASCII 0x46 0x47 0x48 0x49 0x4A)
 *   - Channel:      76
 *   - Payload size: 32 bytes
 *   - Data rate:    1 Mbps   (nRF24 power-on default)
 *   - CRC:          1 byte, enabled (nRF24 power-on default)
 *
 * In idf.py menuconfig for this project, under "Example Configuration":
 *   - Select "Sender"
 *   - Set "GPIO number of MOSI/MISO/SCLK/CS/CE" per your wiring if the
 *     mirf component's Kconfig exposes them separately from the defaults
 *     above (nopnop2002's mirf component usually reads these from Kconfig,
 *     not hardcoded in this file — check CONFIG_MOSI_GPIO etc. in your
 *     sdkconfig; the pin table above is what YOU wired, so make sure
 *     Kconfig matches it)
 *   - Set Radio Channel = 76
 *   - Do NOT enable "Advanced" unless you also update the STM32 side to match
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "mirf.h"

#define TEST_CHANNEL 76
#define TEST_PAYLOAD 32

static const char *TAG = "SENDER";

void sender_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Start");

    NRF24_t dev;
    Nrf24_init(&dev);

    uint8_t payload = TEST_PAYLOAD;
    uint8_t channel = TEST_CHANNEL;
    Nrf24_config(&dev, channel, payload);

    // Destination address must match the STM32's open_rx_pipe address exactly
    esp_err_t ret = Nrf24_setTADDR(&dev, (uint8_t *)"FGHIJ");
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nrf24l01 not installed / SPI wiring wrong");
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    Nrf24_printDetails(&dev);
    ESP_LOGI(TAG, "Sending test packets on channel %d...", channel);

    uint8_t buf[TEST_PAYLOAD];
    uint32_t counter = 0;

    while (1) {
        memset(buf, 0, sizeof(buf));
        snprintf((char *)buf, sizeof(buf), "PING %" PRIu32, counter);

        Nrf24_send(&dev, buf);
        ESP_LOGI(TAG, "Waiting for send result...");

        if (Nrf24_isSend(&dev, 1000)) {
            ESP_LOGI(TAG, "Send OK: %s", buf);
        } else {
            ESP_LOGW(TAG, "Send FAILED (no ACK / timeout): %s", buf);
        }

        counter++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    xTaskCreate(&sender_task, "SENDER", 1024 * 4, NULL, 5, NULL);
}