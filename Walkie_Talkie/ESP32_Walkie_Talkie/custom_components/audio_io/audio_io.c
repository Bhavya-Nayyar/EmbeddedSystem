#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "audio_io.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "audio_io";

i2s_chan_handle_t tx_handle = NULL;
i2s_chan_handle_t rx_handle = NULL;
QueueHandle_t audio_queue = NULL;

void audio_io_init(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle));

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(I2S_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
                        I2S_DATA_BIT_WIDTH_32BIT,
                        I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = GPIO_NUM_33,   
            .ws   = GPIO_NUM_25,   
            .dout = GPIO_NUM_22,   
            .din  = GPIO_NUM_32,   
            .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false },
        },
    };
   
    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_handle));

    ESP_LOGI(TAG, "audio_io initialized (duplex, shared BCLK/WS)");
}

void mic_task(void *pvParameters)
{
    (void)pvParameters;
    while (1) {
        audio_frame_t frame;
        size_t bytes_read = 0;
        esp_err_t err = i2s_channel_read(rx_handle, frame.samples,
                                          sizeof(frame.samples),
                                          &bytes_read, portMAX_DELAY);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "mic read failed: %s", esp_err_to_name(err));
            continue;
        }
        if (xQueueSend(audio_queue, &frame, 0) != pdTRUE) {
            ESP_LOGW(TAG, "audio_queue full, dropping frame");
        }
    }
}

void speaker_task(void *pvParameters)
{
    (void)pvParameters;
    while (1) {
        audio_frame_t frame;
        if (xQueueReceive(audio_queue, &frame, portMAX_DELAY) != pdTRUE) {
            continue;
        }
        size_t bytes_written = 0;
        esp_err_t err = i2s_channel_write(tx_handle, frame.samples,
                                           sizeof(frame.samples),
                                           &bytes_written, portMAX_DELAY);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "speaker write failed: %s", esp_err_to_name(err));
        }
    }
}