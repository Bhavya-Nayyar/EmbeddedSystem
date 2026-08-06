#include "dht.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "stdio.h"
#include "include/DHT11_Sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "SD_Card.h"
#include "esp_timer.h"
#include "Sensor_Data.h"

#define SENSOR_TYPE     DHT_TYPE_DHT11
#define GPIO_PIN        GPIO_NUM_4

static const char *TAG = "DHT11";

QueueHandle_t dht_q_hndl;

void dht11_sensor(void* PvParameters) { 
    gpio_set_pull_mode(GPIO_PIN, GPIO_PULLUP_ONLY);
    gpio_set_direction(GPIO_PIN, GPIO_MODE_INPUT);

    DHT11_Data_t dht11_data;

    while(1) {

        if(dht_read_float_data(SENSOR_TYPE, GPIO_PIN, &dht11_data.humidity, &dht11_data.temperature) == ESP_OK) {
            ESP_LOGI(TAG, "DHT11 Sensor online");

            xSemaphoreTake(sensor_mutex, portMAX_DELAY);

            sensor_data.temperature = dht11_data.temperature;
            sensor_data.humidity = dht11_data.humidity;

            xSemaphoreGive(sensor_mutex);

            if (xQueueSend(dht_q_hndl, &dht11_data, 0) != pdPASS)
            {
                ESP_LOGW(TAG, "Queue Full");
            }

            log_event_t log_evt = {
                .source = LOG_SRC_DHT11,
                .timestamp_us = esp_timer_get_time(),
                .data.dht11.temperature_c = dht11_data.temperature,
                .data.dht11.humidity_pct = dht11_data.humidity,
            };
            
            if (xQueueSend(sd_log_q_hndl, &log_evt, pdMS_TO_TICKS(50)) != pdPASS)
            {
                ESP_LOGW(TAG, "Log queue full, dropped DHT11 reading");
            }
        } else {
            ESP_LOGE(TAG, "Unable to read DHT11 Sensor");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}