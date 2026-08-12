#include "LED.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define LED_GPIO GPIO_NUM_22

static const char *TAG = "LED";

static const int blink_ms_by_version[] = {
    0,
    1000,  
    500,   
    200,   
    50     
};

static void led_task(void *arg)
{
    int fw_version = (int)(intptr_t)arg;

    int max_index = (sizeof(blink_ms_by_version) / sizeof(blink_ms_by_version[0])) - 1;
    if (fw_version < 1) fw_version = 1;
    if (fw_version > max_index) fw_version = max_index;

    int delay_ms = blink_ms_by_version[fw_version];

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    ESP_LOGI(TAG, "FW v%d blink rate = %d ms", fw_version, delay_ms);

    bool led_on = false;
    while (1)
    {
        led_on = !led_on;
        gpio_set_level(LED_GPIO, led_on ? 1 : 0);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

void led_start_task(int fw_version)
{
    xTaskCreate(led_task, "led_task", 2048, (void *)(intptr_t)fw_version, 1, NULL);
}
