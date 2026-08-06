#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "HTTP_Client.h"
#include "Sensor_Data.h"

void upload_task(void *pvParameters)
{
    Sensor_Data_t local;

    while (1)
    {
        xSemaphoreTake(sensor_mutex, portMAX_DELAY);

        local = sensor_data;

        xSemaphoreGive(sensor_mutex);

        http_send_data(local.temperature,
                       local.humidity,
                       local.thermistor,
                       local.pir_motion,
                       local.sound_detected);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}