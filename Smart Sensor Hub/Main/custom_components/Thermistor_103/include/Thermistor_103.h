#ifndef Thermistor_103_H
#define Thermistor_103_H

#include "esp_adc/adc_oneshot.h"
#include "soc/adc_channel.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "math.h"

extern QueueHandle_t therm_q_hndl;

void therm_temp(void *pvParameters);

#endif
