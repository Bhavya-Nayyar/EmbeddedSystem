#ifndef DHT11_Sensor_H
#define DHT11_Sensor_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

typedef struct {
    float temperature, humidity;
} DHT11_Data_t;

extern QueueHandle_t dht_q_hndl;

void dht11_sensor(void* pvParameters);

#endif