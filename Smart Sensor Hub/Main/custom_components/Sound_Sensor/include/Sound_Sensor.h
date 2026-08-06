#ifndef Sound_Sensor_H
#define Sound_Sensor_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdbool.h>

typedef struct
{
    bool sound;
} Sound_Data_t;

extern QueueHandle_t sound_lcd_q_hndl;

void sound_sensor(void *pvParameters);

#endif