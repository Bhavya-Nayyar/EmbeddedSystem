#include "Thermistor_103.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
#include "math.h"
#include "SD_Card.h"
#include "esp_timer.h"
#include "Sensor_Data.h"

#define ADC_CHANNEL ADC_CHANNEL_4
#define FIXED_RESISTOR 10000.0f
#define BETA 3950.0f
#define ROOM_TEMP_K 298.15f
#define ROOM_RESISTANCE 10000.0f
#define VCC 3.3f

static const char *TAG = "THERMISTOR";

QueueHandle_t therm_q_hndl;

static adc_oneshot_unit_handle_t adc_handle;
static adc_cali_handle_t cali_handle;

static void adc_init(void)
{
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_10,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL, &chan_cfg));

    adc_cali_line_fitting_config_t cali_cfg = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_10,
    };

    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_cfg, &cali_handle));
}

static float adc_voltage(void)
{
    int raw;
    int voltage_mv;

    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL, &raw));
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, raw, &voltage_mv));

    return voltage_mv / 1000.0f;
}

void therm_temp(void *pvParameters)
{
    adc_init();

    while (1)
    {
        float vout = adc_voltage();

        if (vout <= 0.0f || vout >= VCC)
        {
            ESP_LOGE(TAG, "Invalid voltage: %.3f V", vout);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        float r_therm = (FIXED_RESISTOR * vout) / (VCC - vout);

        if (r_therm <= 0.0f)
        {
            ESP_LOGE(TAG, "Invalid resistance");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        float temp_k = (BETA * ROOM_TEMP_K) / (BETA + ROOM_TEMP_K * logf(r_therm / ROOM_RESISTANCE));

        float temp_c = temp_k - 273.15f;

        xSemaphoreTake(sensor_mutex, portMAX_DELAY);

        sensor_data.thermistor = temp_c;

        xSemaphoreGive(sensor_mutex);

        log_event_t log_evt = {
            .source = LOG_SRC_THERMISTOR,
            .timestamp_us = esp_timer_get_time(),
            .data.thermistor.temperature_c = temp_c,
        };

        if (xQueueSend(sd_log_q_hndl, &log_evt, pdMS_TO_TICKS(50)) != pdPASS)
        {
            ESP_LOGW(TAG, "Log queue full, dropped thermistor reading");
        }

        xQueueSend(therm_q_hndl, &temp_c, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}