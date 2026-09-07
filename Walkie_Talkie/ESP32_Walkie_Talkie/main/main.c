#include "audio_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    tx_radio_queue = xQueueCreate(32, sizeof(nrf_packet_t));
    rx_radio_queue = xQueueCreate(32, sizeof(nrf_packet_t));

    configASSERT(tx_radio_queue != NULL);
    configASSERT(rx_radio_queue != NULL);

    audio_io_init();
    nrf24_radio_init();

    // Pin Tasks to Cores to prevent cache thrashing & priority inversion
    xTaskCreatePinnedToCore(mic_task, "mic_task", 3072, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(speaker_task, "speaker_task", 3072, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(nrf_radio_task, "nrf_radio_task", 3072, NULL, 5, NULL, 1);
}