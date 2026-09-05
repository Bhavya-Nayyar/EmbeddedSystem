#include <stdio.h>
#include <string.h>

#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_err.h"

#include "HTTP_Client.h"

static const char *TAG = "HTTP";

void http_send_data(float temperature,
                    float humidity,
                    float thermistor,
                    bool pir_motion,
                    bool sound_detected)
{
    char post_data[128];

    snprintf(post_data,
             sizeof(post_data),
             "{\"temperature\":%.2f,"
             "\"humidity\":%.2f,"
             "\"thermistor\":%.2f,"
             "\"pir\":%d,"
             "\"sound\":%d}",
             temperature,
             humidity,
             thermistor,
             pir_motion,
             sound_detected);

    ESP_LOGI(TAG, "Sending: %s", post_data);

    esp_http_client_config_t config = {
        .url = "http://<YOUR IP>:3000/data",
        .timeout_ms = 1000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_method(client, HTTP_METHOD_POST);

    esp_http_client_set_header(client,
                               "Content-Type",
                               "application/json");

    esp_http_client_set_post_field(client,
                                   post_data,
                                   strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG,
                 "HTTP POST Success (Status=%d)",
                 esp_http_client_get_status_code(client));
    }
    else
    {
        ESP_LOGE(TAG,
                 "HTTP POST Failed: %s",
                 esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}