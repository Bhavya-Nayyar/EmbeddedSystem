#include "OLED.h"
#include "driver/i2c_master.h"
#include "ssd1306.h"
#include "esp_log.h"

#define OLED_SDA_GPIO GPIO_NUM_19
#define OLED_SCL_GPIO GPIO_NUM_18
#define OLED_I2C_PORT I2C_NUM_0
#define OLED_I2C_ADDR 0x3C

static const char *TAG = "OLED";

static i2c_master_bus_handle_t s_i2c_bus = NULL;
static ssd1306_handle_t s_ssd_handle = NULL;

static void i2c_bus_init(void)
{
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = OLED_I2C_PORT,
        .sda_io_num = OLED_SDA_GPIO,
        .scl_io_num = OLED_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &s_i2c_bus));
}

void oled_init_bus(void)
{
    i2c_bus_init();

    esp_err_t probe = i2c_master_probe(
        s_i2c_bus,
        OLED_I2C_ADDR,
        1000);

    ESP_LOGI(TAG, "OLED probe result: %s", esp_err_to_name(probe));

    if (probe != ESP_OK)
    {
        ESP_LOGE(TAG, "OLED cannot be reached from OLED I2C bus");
        return;
    }

    ssd1306_config_t ssd_cfg = I2C_SSD1306_128x32_CONFIG_DEFAULT;
    ssd_cfg.display_enabled = true;

    esp_err_t err = ssd1306_init(
        s_i2c_bus,
        &ssd_cfg,
        &s_ssd_handle);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "ssd1306_init failed: %s", esp_err_to_name(err));
        s_ssd_handle = NULL;
        return;
    }

    ESP_ERROR_CHECK(ssd1306_clear_display(s_ssd_handle, 0));

    ESP_LOGI(TAG, "OLED initialized");
}

void oled_show_status(const char *line1, const char *line2)
{
    if (s_ssd_handle == NULL)
    {
        ESP_LOGW(
            TAG,
            "OLED not initialized, skipping: %s / %s",
            line1 ? line1 : "",
            line2 ? line2 : "");

        return;
    }

    ESP_ERROR_CHECK(
        ssd1306_clear_display(s_ssd_handle, 0));

    if (line1 != NULL)
    {
        ESP_ERROR_CHECK(
            ssd1306_display_text(
                s_ssd_handle,
                0,
                line1,
                false));
    }

    if (line2 != NULL)
    {
        ESP_ERROR_CHECK(
            ssd1306_display_text(
                s_ssd_handle,
                1,
                line2,
                false));
    }
}