#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "ssd1306.h"
#include "DHT11_Sensor.h"
#include "Thermistor_103.h"

static ssd1306_handle_t oled_init(i2c_master_bus_handle_t mstr_bus_hndl) {
    ssd1306_config_t ssd_cnfg = I2C_SSD1306_128x32_CONFIG_DEFAULT;

    ssd1306_handle_t ssd_hndl = NULL;

    ESP_ERROR_CHECK(ssd1306_init(mstr_bus_hndl, &ssd_cnfg, &ssd_hndl));

    return ssd_hndl;
}

void oled_display_temp_hum(void *pvParameters) {
    i2c_master_bus_handle_t shared_bus = (i2c_master_bus_handle_t)pvParameters;

    ssd1306_handle_t ssd_hndl = oled_init(shared_bus);

    ESP_ERROR_CHECK(ssd1306_clear_display(ssd_hndl, 0));

    DHT11_Data_t dht11_data;

    float t_celsius;

    char temp_buf[16];
    char hum_buf[16];

    while (1)
    {
        if (xQueueReceive(dht_q_hndl, &dht11_data, 0) == pdTRUE && xQueueReceive(therm_q_hndl, &t_celsius, 0) == pdTRUE) {
            float avg_temp = (dht11_data.temperature + t_celsius) / 2;

            snprintf(temp_buf, sizeof(temp_buf), "Temp: %.1f C", avg_temp);
            snprintf(hum_buf, sizeof(hum_buf), "Hum: %.1f %%", dht11_data.humidity);

            ESP_ERROR_CHECK(ssd1306_display_text(ssd_hndl, 0, temp_buf, false));
            ESP_ERROR_CHECK(ssd1306_display_text(ssd_hndl, 1, hum_buf, false));
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}