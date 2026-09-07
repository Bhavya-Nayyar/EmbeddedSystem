#include "audio_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    audio_queue = xQueueCreate(8, sizeof(audio_frame_t));
    configASSERT(audio_queue != NULL);

    audio_io_init();   

    xTaskCreate(mic_task,     "mic_task",     4096, NULL, 6, NULL);
    xTaskCreate(speaker_task, "speaker_task", 4096, NULL, 5, NULL);
}