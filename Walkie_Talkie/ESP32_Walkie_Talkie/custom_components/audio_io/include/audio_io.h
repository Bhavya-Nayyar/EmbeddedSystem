#ifndef AUDIO_IO_H
#define AUDIO_IO_H

#include <stdint.h>
#include "driver/i2s_std.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define I2S_SAMPLE_RATE       16000
#define AUDIO_FRAME_SAMPLES   256
#define NRF_PAYLOAD_SIZE      32

#define NRF_CE_GPIO            4
#define NRF_CSN_GPIO           5

#define NRF_SPI_SCK            18
#define NRF_SPI_MISO           19
#define NRF_SPI_MOSI           23

typedef struct {
    int32_t samples[AUDIO_FRAME_SAMPLES];
} audio_frame_t;

typedef struct {
    uint8_t data[NRF_PAYLOAD_SIZE];
} nrf_packet_t;

extern i2s_chan_handle_t tx_handle;
extern i2s_chan_handle_t rx_handle;

extern QueueHandle_t tx_radio_queue;
extern QueueHandle_t rx_radio_queue;

void audio_io_init(void);
void nrf24_radio_init(void);

void mic_task(void *pvParameters);
void speaker_task(void *pvParameters);
void nrf_radio_task(void *pvParameters);

#endif