#include "SD_Card.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "esp_timer.h"
#include "esp_log.h"

#define PIN_MISO GPIO_NUM_21
#define PIN_MOSI GPIO_NUM_25
#define PIN_SCK GPIO_NUM_14
#define PIN_CS GPIO_NUM_13

static const char *TAG = "SD_CARD";
static sdmmc_card_t *card;

QueueHandle_t sd_log_q_hndl = NULL;

static esp_err_t sd_init(void)
{
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
    };

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = PIN_MISO,
        .sclk_io_num = PIN_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST;
    host.max_freq_khz = 4000; 

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_CS;
    slot_config.host_id = SPI2_HOST;

    ret = esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Mount failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "SD card mounted");
    sdmmc_card_print_info(stdout, card);
    return ESP_OK;
}

void sd_logger_task(void *pvParameters)
{
    if (sd_init() != ESP_OK)
    {
        ESP_LOGE(TAG, "SD init failed, logger task exiting");
        vTaskDelete(NULL);
        return;
    }

    log_event_t evt;

    while (1)
    {
        if (xQueueReceive(sd_log_q_hndl, &evt, portMAX_DELAY) == pdTRUE)
        {
            FILE *f = fopen("/sdcard/sensor_log.csv", "a");
            if (f == NULL)
            {
                ESP_LOGE(TAG, "Failed to open log file");
                continue;
            }

            switch (evt.source)
            {
            case LOG_SRC_SOUND:
                fprintf(f, "%lld,SOUND,%d\n",
                        evt.timestamp_us, evt.data.sound.detected);
                break;
            case LOG_SRC_PIR:
                fprintf(f, "%lld,PIR,%d\n",
                        evt.timestamp_us, evt.data.pir.motion);
                break;
            case LOG_SRC_DHT11:
                fprintf(f, "%lld,DHT11,%.1f,%.1f\n",
                        evt.timestamp_us,
                        evt.data.dht11.temperature_c,
                        evt.data.dht11.humidity_pct);
                break;
            case LOG_SRC_THERMISTOR:
                fprintf(f, "%lld,THERMISTOR,%.2f\n",
                        evt.timestamp_us,
                        evt.data.thermistor.temperature_c);
                break;
            default:
                ESP_LOGW(TAG, "Unknown log source %d", evt.source);
                break;
            }

            fclose(f);
        }
    }
}