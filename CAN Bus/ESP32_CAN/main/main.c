#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include "stdio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "ESP_CAN";
static twai_node_handle_t node_hdl = NULL;

static bool IRAM_ATTR twai_rx_cb( twai_node_handle_t handle, const twai_rx_done_event_data_t *edata, void *user_ctx) {
    uint8_t recv_buff[8];

    twai_frame_t rx_frame = {
        .buffer = recv_buff,
        .buffer_len = sizeof(recv_buff),
    };

    if (twai_node_receive_from_isr(handle, &rx_frame) == ESP_OK) {

        ESP_DRAM_LOGI(TAG, "RX id=0x%03lX dlc=%u", rx_frame.header.id, rx_frame.header.dlc);

        ESP_DRAM_LOGI(TAG, "RX DATA: %02X %02X %02X %02X %02X %02X %02X %02X", recv_buff[0], recv_buff[1], recv_buff[2], recv_buff[3], 
                      recv_buff[4], recv_buff[5], recv_buff[6],recv_buff[7]
                     );
    }

    return false;
}

static bool IRAM_ATTR twai_state_change_cb(
    twai_node_handle_t handle,
    const twai_state_change_event_data_t *edata,
    void *user_ctx)
{
    ESP_DRAM_LOGI(TAG, "CAN state changed: %d", edata->new_sta);

    if (edata->new_sta == TWAI_ERROR_BUS_OFF) {
        ESP_DRAM_LOGE(TAG, "CAN BUS OFF - recovering");

        esp_err_t err = twai_node_recover(handle);

        if (err != ESP_OK) {
            ESP_DRAM_LOGE(TAG,"CAN recovery failed: %s", esp_err_to_name(err));
        }
    }

    return false;
}

void app_main(void)
{
    twai_onchip_node_config_t node_config = {
        .io_cfg.tx = 22,
        .io_cfg.rx = 21,

        .bit_timing.bitrate = 250000,

        .tx_queue_depth = 5,
    };

    ESP_ERROR_CHECK(twai_new_node_onchip(&node_config, &node_hdl));

    twai_event_callbacks_t user_cbs = {
        .on_rx_done = twai_rx_cb,
        .on_state_change = twai_state_change_cb,
    };

    ESP_ERROR_CHECK(twai_node_register_event_callbacks(node_hdl, &user_cbs, NULL));

    twai_mask_filter_config_t rx_filter = {
        .id = 0x101,
        .mask = 0x7FF,
        .is_ext = false,
    };

    ESP_ERROR_CHECK(twai_node_config_mask_filter(node_hdl, 0, &rx_filter));

    ESP_ERROR_CHECK(twai_node_enable(node_hdl));

    ESP_LOGI(TAG, "ESP_CAN duplex mode enabled");
    ESP_LOGI(TAG, "Bitrate: 250000 bit/s");
    ESP_LOGI(TAG, "RX ID: 0x101");
    ESP_LOGI(TAG, "TX ID: 0x100");

    uint8_t send_buff[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

    twai_frame_t tx_msg = {
        .header.id = 0x100,
        .buffer = send_buff,
        .buffer_len = sizeof(send_buff),
    };

    uint32_t counter = 0;

    while (1)
    {
        send_buff[0] = (uint8_t)(counter & 0xFF);

        esp_err_t err = twai_node_transmit(node_hdl, &tx_msg, 0);

        if (err != ESP_OK) {

            ESP_LOGE(TAG, "TX QUEUE FAILED: %s", esp_err_to_name(err));
        } else
        {
            err = twai_node_transmit_wait_all_done(node_hdl, pdMS_TO_TICKS(1000));

            if (err == ESP_OK) {

                ESP_LOGI(TAG, "TX SUCCESS #%lu", counter);

                ESP_LOGI(TAG, "TX DATA: %02X %02X %02X %02X %02X %02X %02X %02X",
                         send_buff[0],
                         send_buff[1],
                         send_buff[2],
                         send_buff[3],
                         send_buff[4],
                         send_buff[5],
                         send_buff[6],
                         send_buff[7]
                        );
            } else {

                ESP_LOGE(TAG, "TX COMPLETE FAILED #%lu: %s", counter, esp_err_to_name(err));
            }
        }

        twai_node_status_t status;
        twai_node_record_t stats;

        err = twai_node_get_info(node_hdl, &status, &stats);

        if (err == ESP_OK) {

            ESP_LOGI(TAG, "state=%d tx_err=%u rx_err=%u bus_err_num=%lu", status.state, status.tx_error_count, status.rx_error_count, stats.bus_err_num);
        } else {

            ESP_LOGE(TAG, "Failed to get CAN status: %s", esp_err_to_name(err));
        }

        counter++;

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}