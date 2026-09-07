#include "audio_io.h"
#include <string.h>
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/task.h"

static const char *TAG = "AUDIO_IO";

i2s_chan_handle_t tx_handle = NULL;
i2s_chan_handle_t rx_handle = NULL;

QueueHandle_t tx_radio_queue = NULL;
QueueHandle_t rx_radio_queue = NULL;

static spi_device_handle_t nrf_spi_handle = NULL;

#define NRF_CONFIG       0x00
#define NRF_EN_AA        0x01
#define NRF_EN_RXADDR    0x02
#define NRF_SETUP_AW     0x03
#define NRF_SETUP_RETR   0x04
#define NRF_RF_CH        0x05
#define NRF_RF_SETUP     0x06
#define NRF_STATUS       0x07

#define NRF_RX_ADDR_P0   0x0A
#define NRF_TX_ADDR      0x10
#define NRF_RX_PW_P0     0x11

#define NRF_R_REGISTER   0x00
#define NRF_W_REGISTER   0x20
#define NRF_R_RX_PAYLOAD 0x61
#define NRF_W_TX_PAYLOAD 0xA0
#define NRF_FLUSH_TX     0xE1
#define NRF_FLUSH_RX     0xE2
#define NRF_NOP          0xFF

#define NRF_STATUS_RX_DR (1 << 6)
#define NRF_STATUS_TX_DS (1 << 5)
#define NRF_STATUS_MAX_RT (1 << 4)

static const uint8_t NRF_ADDRESS[5] = {0xD2, 0xD2, 0xD2, 0xD2, 0xD2};

static uint8_t nrf_read_register(uint8_t reg) {
    uint8_t tx[2] = { NRF_R_REGISTER | (reg & 0x1F), NRF_NOP };
    uint8_t rx[2] = {0};
    spi_transaction_t t = { .length = 16, .tx_buffer = tx, .rx_buffer = rx };

    gpio_set_level(NRF_CSN_GPIO, 0);
    spi_device_transmit(nrf_spi_handle, &t);
    gpio_set_level(NRF_CSN_GPIO, 1);
    return rx[1];
}

static void nrf_write_register(uint8_t reg, uint8_t value) {
    uint8_t tx[2] = { NRF_W_REGISTER | (reg & 0x1F), value };
    spi_transaction_t t = { .length = 16, .tx_buffer = tx };

    gpio_set_level(NRF_CSN_GPIO, 0);
    spi_device_transmit(nrf_spi_handle, &t);
    gpio_set_level(NRF_CSN_GPIO, 1);
}

static void nrf_write_register_multi(uint8_t reg, const uint8_t *data, size_t length) {
    uint8_t buffer[6];
    if (length > 5) return;
    buffer[0] = NRF_W_REGISTER | (reg & 0x1F);
    memcpy(&buffer[1], data, length);

    spi_transaction_t t = { .length = (length + 1) * 8, .tx_buffer = buffer };

    gpio_set_level(NRF_CSN_GPIO, 0);
    spi_device_transmit(nrf_spi_handle, &t);
    gpio_set_level(NRF_CSN_GPIO, 1);
}

static void nrf_command(uint8_t command) {
    spi_transaction_t t = { .length = 8, .tx_buffer = &command };

    gpio_set_level(NRF_CSN_GPIO, 0);
    spi_device_transmit(nrf_spi_handle, &t);
    gpio_set_level(NRF_CSN_GPIO, 1);
}

void audio_io_init(void) {
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle));

    i2s_std_config_t std_cfg = {
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(I2S_SAMPLE_RATE),
    .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
    .gpio_cfg = {
        .mclk = I2S_GPIO_UNUSED,
        .bclk = GPIO_NUM_33,
        .ws   = GPIO_NUM_25,
        .dout = GPIO_NUM_22,
        .din  = GPIO_NUM_32,
        .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false }
    }
};
    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_handle));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));
}

void nrf24_radio_init(void) {
    gpio_reset_pin(NRF_CE_GPIO);
    gpio_set_direction(NRF_CE_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(NRF_CE_GPIO, 0);

    gpio_reset_pin(NRF_CSN_GPIO);
    gpio_set_direction(NRF_CSN_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(NRF_CSN_GPIO, 1);

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = NRF_SPI_MOSI,
        .miso_io_num = NRF_SPI_MISO,
        .sclk_io_num = NRF_SPI_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 64
    };

    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = 8 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = -1,
        .queue_size = 7
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev_cfg, &nrf_spi_handle));

    nrf_write_register(NRF_CONFIG, 0x08);
    nrf_write_register(NRF_EN_AA, 0x00);
    nrf_write_register(NRF_EN_RXADDR, 0x01);
    nrf_write_register(NRF_SETUP_AW, 0x03);
    nrf_write_register(NRF_SETUP_RETR, 0x00);
    nrf_write_register(NRF_RF_CH, 76);
    nrf_write_register(NRF_RF_SETUP, 0x0E); // 2Mbps, 0dBm

    nrf_write_register_multi(NRF_RX_ADDR_P0, NRF_ADDRESS, 5);
    nrf_write_register_multi(NRF_TX_ADDR, NRF_ADDRESS, 5);
    nrf_write_register(NRF_RX_PW_P0, NRF_PAYLOAD_SIZE);

    nrf_write_register(NRF_STATUS, NRF_STATUS_RX_DR | NRF_STATUS_TX_DS | NRF_STATUS_MAX_RT);
    nrf_command(NRF_FLUSH_TX);
    nrf_command(NRF_FLUSH_RX);

    nrf_write_register(NRF_CONFIG, 0x0B); // RX Mode
    esp_rom_delay_us(150);
    gpio_set_level(NRF_CE_GPIO, 1);
}

static bool nrf24_transmit(const uint8_t *payload) {
    uint8_t tx_buffer[NRF_PAYLOAD_SIZE + 1];
    tx_buffer[0] = NRF_W_TX_PAYLOAD;
    memcpy(&tx_buffer[1], payload, NRF_PAYLOAD_SIZE);

    gpio_set_level(NRF_CE_GPIO, 0);
    nrf_write_register(NRF_CONFIG, 0x0A); // TX Mode
    esp_rom_delay_us(130);

    nrf_write_register(NRF_STATUS, NRF_STATUS_TX_DS | NRF_STATUS_MAX_RT);
    nrf_command(NRF_FLUSH_TX);

    spi_transaction_t t = { .length = sizeof(tx_buffer) * 8, .tx_buffer = tx_buffer };
    gpio_set_level(NRF_CSN_GPIO, 0);
    spi_device_transmit(nrf_spi_handle, &t);
    gpio_set_level(NRF_CSN_GPIO, 1);

    gpio_set_level(NRF_CE_GPIO, 1);
    esp_rom_delay_us(15); // >10us high pulse
    gpio_set_level(NRF_CE_GPIO, 0);

    // Microsecond spin-wait for TX end
    uint32_t timeout = 1000;
    while (timeout--) {
        uint8_t status = nrf_read_register(NRF_STATUS);
        if (status & NRF_STATUS_TX_DS) {
            nrf_write_register(NRF_STATUS, NRF_STATUS_TX_DS);
            return true;
        }
        if (status & NRF_STATUS_MAX_RT) {
            nrf_write_register(NRF_STATUS, NRF_STATUS_MAX_RT);
            return false;
        }
        esp_rom_delay_us(10);
    }
    return false;
}

static bool nrf24_receive(uint8_t *payload) {
    uint8_t status = nrf_read_register(NRF_STATUS);
    if (!(status & NRF_STATUS_RX_DR)) return false;

    uint8_t tx_buf[NRF_PAYLOAD_SIZE + 1] = { NRF_R_RX_PAYLOAD };
    uint8_t rx_buf[NRF_PAYLOAD_SIZE + 1] = {0};

    spi_transaction_t t = { .length = sizeof(tx_buf) * 8, .tx_buffer = tx_buf, .rx_buffer = rx_buf };

    gpio_set_level(NRF_CSN_GPIO, 0);
    spi_device_transmit(nrf_spi_handle, &t);
    gpio_set_level(NRF_CSN_GPIO, 1);

    memcpy(payload, &rx_buf[1], NRF_PAYLOAD_SIZE);
    nrf_write_register(NRF_STATUS, NRF_STATUS_RX_DR);
    return true;
}

void mic_task(void *pvParameters) {
    (void)pvParameters;
    audio_frame_t frame;

    while (1) {
        size_t bytes_read = 0;
        if (i2s_channel_read(rx_handle, frame.samples, sizeof(frame.samples), &bytes_read, portMAX_DELAY) == ESP_OK) {
            for (int packet_index = 0; packet_index < 8; packet_index++) {
                nrf_packet_t packet;
                for (int i = 0; i < NRF_PAYLOAD_SIZE; i++) {
                    int index = packet_index * NRF_PAYLOAD_SIZE + i;
                    
                    // Shift right by 23 to keep sign bit intact and extract top 8 bits
                    int32_t sample = frame.samples[index] >> 23; 
                    
                    // Convert signed int8 (-128 to 127) to unsigned uint8 (0 to 255)
                    packet.data[i] = (uint8_t)((int8_t)sample + 128);
                }
                xQueueSend(tx_radio_queue, &packet, 0);
            }
        }
    }
}

void speaker_task(void *pvParameters) {
    (void)pvParameters;
    nrf_packet_t packet;
    int32_t output[NRF_PAYLOAD_SIZE];

    while (1) {
        if (xQueueReceive(rx_radio_queue, &packet, portMAX_DELAY) == pdTRUE) {
            for (int i = 0; i < NRF_PAYLOAD_SIZE; i++) {
                int8_t sample8 = (int8_t)(packet.data[i] - 128);
                output[i] = ((int32_t)sample8) << 24;
            }
            size_t bytes_written = 0;
            i2s_channel_write(tx_handle, output, sizeof(output), &bytes_written, portMAX_DELAY);
        }
    }
}

void nrf_radio_task(void *pvParameters) {
    (void)pvParameters;
    nrf_packet_t packet;

    while (1) {
        // Process TX Priority Queue
        if (xQueueReceive(tx_radio_queue, &packet, 0) == pdTRUE) {
            nrf24_transmit(packet.data);

            // Return to RX Mode safely
            gpio_set_level(NRF_CE_GPIO, 0);
            nrf_write_register(NRF_CONFIG, 0x0B);
            esp_rom_delay_us(130);
            gpio_set_level(NRF_CE_GPIO, 1);
        }

        // Process RX Buffer
        if (nrf24_receive(packet.data)) {
            xQueueSend(rx_radio_queue, &packet, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}