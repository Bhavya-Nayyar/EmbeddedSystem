#include "FOTA.h"
#include "OLED.h"
#include "esp_http_client.h"
#include "esp_ota_ops.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdbool.h>

static const char *TAG = "FOTA";

#define OTA_BUFFER 1024

/* Set by ota_func just before the deliberate WiFi-stop + restart,
   so main.c's WiFi event handler can check it and skip the
   reconnect-on-disconnect logic during shutdown. Declared extern
   here; main.c owns the actual variable. */
extern volatile bool g_ota_restarting;

void fota_validate_running_app(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;

    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK)
    {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY)
        {
            /* Replace with a real self-test (sensor init OK, WiFi
               reachable, etc) if you want actual rollback protection
               rather than an unconditional pass. */
            bool self_test_passed = true;

            if (self_test_passed)
            {
                ESP_LOGI(TAG, "New image passed self-test, marking valid");
                esp_ota_mark_app_valid_cancel_rollback();
            }
            else
            {
                ESP_LOGE(TAG, "Self-test failed, rolling back");
                esp_ota_mark_app_invalid_rollback_and_reboot();
                /* does not return */
            }
        }
    }
}

void fota_check_and_update(void)
{
    const esp_partition_t *running_partition = esp_ota_get_running_partition();
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);

    if (running_partition == NULL)
    {
        ESP_LOGE(TAG, "No running partition found");
        oled_show_status("OTA FAILED", "no running part");
        return;
    }

    if (update_partition == NULL)
    {
        ESP_LOGE(TAG, "No OTA update partition found");
        oled_show_status("OTA FAILED", "no update part");
        return;
    }

    ESP_LOGI(TAG, "Running: %s, Update: %s", running_partition->label, update_partition->label);
    oled_show_status("OTA: connecting", update_partition->label);

    esp_http_client_config_t http_client_config = {
        .url = CONFIG_OTA_FIRMWARE_URL,
    };

    esp_http_client_handle_t client = esp_http_client_init(&http_client_config);
    if (client == NULL)
    {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        oled_show_status("OTA FAILED", "http init");
        return;
    }

    ESP_ERROR_CHECK(esp_http_client_open(client, 0));

    int64_t content_length = esp_http_client_fetch_headers(client);
    if (content_length < 0)
    {
        ESP_LOGE(TAG, "Failed to fetch HTTP headers");
        oled_show_status("OTA FAILED", "headers");
        esp_http_client_cleanup(client);
        return;
    }

    int status = esp_http_client_get_status_code(client);
    if (status != 200)
    {
        ESP_LOGE(TAG, "HTTP error: status = %d", status);
        oled_show_status("OTA FAILED", "http status");
        esp_http_client_cleanup(client);
        return;
    }

    ESP_LOGI(TAG, "Firmware size: %lld bytes", content_length);
    oled_show_status("OTA: downloading", "0 %");

    esp_ota_handle_t ota_handle;
    ESP_ERROR_CHECK(esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle));

    char buffer[OTA_BUFFER];
    size_t total_written = 0;
    int last_pct_shown = -1;

    while (1)
    {
        int bytes_read = esp_http_client_read(client, buffer, OTA_BUFFER);

        if (bytes_read < 0)
        {
            ESP_LOGE(TAG, "HTTP read error, aborting OTA");
            oled_show_status("OTA FAILED", "read error");
            esp_ota_abort(ota_handle);
            esp_http_client_cleanup(client);
            return;
        }

        if (bytes_read == 0)
        {
            break;
        }

        esp_err_t write_err = esp_ota_write(ota_handle, buffer, bytes_read);
        if (write_err != ESP_OK)
        {
            ESP_LOGE(TAG, "OTA write failed: %s", esp_err_to_name(write_err));
            oled_show_status("OTA FAILED", "write error");
            esp_ota_abort(ota_handle);
            esp_http_client_cleanup(client);
            return;
        }

        total_written += bytes_read;

        if (content_length > 0)
        {
            int pct = (int)((total_written * 100) / (size_t)content_length);
            if (pct != last_pct_shown)
            {
                char pct_str[8];
                snprintf(pct_str, sizeof(pct_str), "%d %%", pct);
                oled_show_status("OTA: downloading", pct_str);
                last_pct_shown = pct;
            }
        }
    }

    ESP_LOGI(TAG, "Total firmware written: %u bytes", (unsigned)total_written);

    if (content_length > 0 && total_written != (size_t)content_length)
    {
        ESP_LOGE(TAG, "Size mismatch: got %u, expected %lld", (unsigned)total_written, content_length);
        oled_show_status("OTA FAILED", "size mismatch");
        esp_ota_abort(ota_handle);
        esp_http_client_cleanup(client);
        return;
    }

    esp_err_t end_err = esp_ota_end(ota_handle);
    if (end_err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_end failed: %s", esp_err_to_name(end_err));
        oled_show_status("OTA FAILED", "validate fail");
        esp_http_client_cleanup(client);
        return;
    }

    ESP_ERROR_CHECK(esp_ota_set_boot_partition(update_partition));

    ESP_LOGI(TAG, "OTA successful. Restarting...");
    oled_show_status("OTA OK", "restarting...");

    esp_http_client_cleanup(client);

    g_ota_restarting = true;
    esp_wifi_stop();
    esp_restart();
}
