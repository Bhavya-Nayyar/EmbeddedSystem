#ifndef PIR_SENSOR_H
#define PIR_SENSOR_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdbool.h>

typedef struct
{
    bool motion;
} PIR_Data_t;

extern QueueHandle_t pir_q_hndl;

void pir_sensor(void *pvParameters);

#endif