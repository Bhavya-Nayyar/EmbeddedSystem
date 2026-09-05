#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include "stdbool.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

typedef struct
{
    float temperature;
    float humidity;
    float thermistor;

    bool pir_motion;
    bool sound_detected;

} Sensor_Data_t;

extern Sensor_Data_t sensor_data;
extern SemaphoreHandle_t sensor_mutex;

#endif