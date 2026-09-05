#ifndef SD_Card_H
#define SD_Card_H

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef enum
{
    LOG_SRC_SOUND,
    LOG_SRC_PIR,
    LOG_SRC_DHT11,
    LOG_SRC_THERMISTOR,
} log_source_t;

typedef struct
{
    log_source_t source;
    int64_t timestamp_us; 
    union
    {
        struct
        {
            bool detected;
        } sound;
        struct
        {
            bool motion;
        } pir;
        struct
        {
            float temperature_c;
            float humidity_pct;
        } dht11;
        struct
        {
            float temperature_c;
        } thermistor;
    } data;
} log_event_t;

extern QueueHandle_t sd_log_q_hndl;

void sd_logger_task(void *pvParameters);

#endif