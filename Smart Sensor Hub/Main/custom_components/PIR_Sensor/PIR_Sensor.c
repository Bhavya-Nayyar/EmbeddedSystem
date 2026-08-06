#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "PIR_Sensor.h"
#include "SD_Card.h"
#include "esp_timer.h"
#include "Sensor_Data.h"

#define PIR_PIN GPIO_NUM_5

static TaskHandle_t pir_task_handle = NULL;

QueueHandle_t pir_q_hndl = NULL;

static void IRAM_ATTR pir_isr(void *arg)
{
    BaseType_t higher_priority_task_woken = pdFALSE;

    vTaskNotifyGiveFromISR(pir_task_handle, &higher_priority_task_woken);

    portYIELD_FROM_ISR(higher_priority_task_woken);
}

static void pir_init(void)
{
    gpio_config_t pir_cnfg = {
        .pin_bit_mask = 1ULL << PIR_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE, 
    };

    ESP_ERROR_CHECK(gpio_config(&pir_cnfg));
}

void pir_sensor(void *pvParameters)
{
    pir_task_handle = xTaskGetCurrentTaskHandle();

    pir_init();

    ESP_ERROR_CHECK(gpio_isr_handler_add(PIR_PIN, pir_isr, NULL));

    PIR_Data_t pir_data;
    bool last_motion_state = false;  

    while (1) 
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 

        vTaskDelay(pdMS_TO_TICKS(20));

        int level = gpio_get_level(PIR_PIN);
        
        bool motion_now = (level == 1);

        if (motion_now != last_motion_state)
        {
            last_motion_state = motion_now;

            pir_data.motion = motion_now;

            xSemaphoreTake(sensor_mutex, portMAX_DELAY);

            sensor_data.pir_motion = motion_now;

            xSemaphoreGive(sensor_mutex);

            if (xQueueSend(pir_q_hndl, &pir_data, pdMS_TO_TICKS(50)) != pdPASS)
            {
                ESP_LOGW("PIR", "Queue full, dropped motion event (state=%d)", motion_now);
            }

            log_event_t log_evt = {
                .source = LOG_SRC_PIR,
                .timestamp_us = esp_timer_get_time(),
                .data.pir.motion = motion_now,
            };
            
            if (xQueueSend(sd_log_q_hndl, &log_evt, pdMS_TO_TICKS(50)) != pdPASS)
            {
                ESP_LOGW("PIR", "Log queue full, dropped motion event");
            }
        }
    }
}