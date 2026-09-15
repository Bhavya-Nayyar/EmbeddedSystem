#ifndef AUDIO_IO_H
#define AUDIO_IO_H

#include "driver/i2s_std.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

extern i2s_chan_handle_t tx_handle;
extern i2s_chan_handle_t rx_handle;
extern QueueHandle_t audio_queue;

#define I2S_SAMPLE_RATE      48000
#define AUDIO_FRAME_SAMPLES  256

typedef struct {
    int32_t samples[AUDIO_FRAME_SAMPLES];
} audio_frame_t;

void audio_io_init(void);

void mic_task(void *pvParameters);
void speaker_task(void *pvParameters);

#endif 