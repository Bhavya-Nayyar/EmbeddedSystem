#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "DHT11_Sensor.h"
#include "Thermistor_103.h"
#include "PIR_Sensor.h"
#include "Sound_Sensor.h"
#include "Temp_Hum_OLED.h"
#include "PIR_LCD.h"
#include "Sound_LCD.h"
#include "SD_Card.h"
#include "Wifi.h"
#include "Http_Client.h"
#include "Upload_Task.h"
#include "Sensor_Data.h"

static const char *TAG = "MAIN"; 

#define SHARED_SDA GPIO_NUM_23
#define SHARED_SCL GPIO_NUM_22

void app_main(void) {
    sensor_mutex = xSemaphoreCreateMutex();

    if (sensor_mutex == NULL)
    {
        abort();
    }

    sd_log_q_hndl = xQueueCreate(20, sizeof(log_event_t));

    dht_q_hndl = xQueueCreate(10, sizeof(DHT11_Data_t));
    therm_q_hndl = xQueueCreate(10, sizeof(float));
    pir_q_hndl = xQueueCreate(10, sizeof(PIR_Data_t));
    sound_lcd_q_hndl = xQueueCreate(10, sizeof(Sound_Data_t));

    if (sd_log_q_hndl == NULL || dht_q_hndl == NULL || therm_q_hndl == NULL || pir_q_hndl == NULL || sound_lcd_q_hndl == NULL)
    {
        ESP_LOGE(TAG, "Queue creation failed, halting");
        abort();
    }

    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    i2c_master_bus_config_t shared_bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = SHARED_SDA,
        .scl_io_num = SHARED_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t shared_bus;

    ESP_ERROR_CHECK(i2c_new_master_bus(&shared_bus_cfg, &shared_bus));

    xTaskCreate(sd_logger_task, "sd_logger", 4096, NULL, 5, NULL);

    xTaskCreate(oled_display_temp_hum, "OLED", 4096, (void *)shared_bus, 1, NULL);
    xTaskCreate(pir_lcd_task, "PIR_LCD", 4096, (void *)shared_bus, 1, NULL);

    xTaskCreate(sound_lcd_task, "SOUND_LCD", 4096, NULL, 1, NULL);

    xTaskCreate(pir_sensor, "PIR", 2048, NULL, 1, NULL);
    xTaskCreate(sound_sensor, "Sound", 2048, NULL, 1, NULL);
    xTaskCreate(dht11_sensor, "DHT11", 4096, NULL, 1, NULL);
    xTaskCreate(therm_temp, "Thermistor", 2048, NULL, 1, NULL);

    xTaskCreate(wifi_init_sta, "wifi", 4096, NULL, 7, NULL);

    xTaskCreate(upload_task, "UPLOAD", 4096, NULL, 5, NULL);
}
