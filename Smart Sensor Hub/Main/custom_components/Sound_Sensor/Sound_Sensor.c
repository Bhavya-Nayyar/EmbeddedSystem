#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "Sound_Sensor.h"
#include "SD_Card.h"
#include "esp_timer.h"
#include "Sensor_Data.h"

#define SOUND_PIN   GPIO_NUM_19

static TaskHandle_t sound_task_handle = NULL;

QueueHandle_t sound_lcd_q_hndl = NULL;

static void IRAM_ATTR sound_isr(void *arg)
{
    BaseType_t higher_priority_task_woken = pdFALSE;

    vTaskNotifyGiveFromISR(sound_task_handle, &higher_priority_task_woken);

    portYIELD_FROM_ISR(higher_priority_task_woken);
}

static void sound_init(void)
{
    gpio_config_t sound_cnfg = {
        .pin_bit_mask = 1ULL << SOUND_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };

    ESP_ERROR_CHECK(gpio_config(&sound_cnfg));
}

void sound_sensor(void *pvParameters)
{
    sound_task_handle = xTaskGetCurrentTaskHandle();

    sound_init();

    ESP_ERROR_CHECK(gpio_isr_handler_add(SOUND_PIN, sound_isr, NULL));

    Sound_Data_t sound_data;
    bool last_sound_state = false;

    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(20));

        int level = gpio_get_level(SOUND_PIN);
        bool sound_now = (level == 1);

        if (sound_now && !last_sound_state)
        {
            last_sound_state = true;

            sound_data.sound = true;

            xSemaphoreTake(sensor_mutex, portMAX_DELAY);

            sensor_data.sound_detected = true;

            xSemaphoreGive(sensor_mutex);

            if (xQueueSend(sound_lcd_q_hndl, &sound_data, pdMS_TO_TICKS(50)) != pdPASS)
            {
                ESP_LOGW("SOUND", "Queue full, dropped sound event (state=1)");
            }

            log_event_t log_evt = {
                .source = LOG_SRC_SOUND,
                .timestamp_us = esp_timer_get_time(),
                .data.sound.detected = true,
            };

            if (xQueueSend(sd_log_q_hndl, &log_evt, pdMS_TO_TICKS(50)) != pdPASS)
            {
                ESP_LOGW("SOUND", "Log queue full, dropped sound event (state=1)");
            }

            vTaskDelay(pdMS_TO_TICKS(2000));

            sound_data.sound = false;

            xSemaphoreTake(sensor_mutex, portMAX_DELAY);

            sensor_data.sound_detected = false;

            xSemaphoreGive(sensor_mutex);

            if (xQueueSend(sound_lcd_q_hndl, &sound_data, pdMS_TO_TICKS(50)) != pdPASS)
            {
                ESP_LOGW("SOUND", "Queue full, dropped sound event (state=0)");
            }

            log_event_t log_evt_clear = {
                .source = LOG_SRC_SOUND,
                .timestamp_us = esp_timer_get_time(),
                .data.sound.detected = false,
            };
            
            if (xQueueSend(sd_log_q_hndl, &log_evt_clear, pdMS_TO_TICKS(50)) != pdPASS)
            {
                ESP_LOGW("SOUND", "Log queue full, dropped sound event (state=0)");
            }

            last_sound_state = (gpio_get_level(SOUND_PIN) == 1);
        }
        else if (!sound_now)
        {
            last_sound_state = false;
        }
    }
}