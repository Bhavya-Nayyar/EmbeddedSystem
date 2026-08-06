#include "PIR_LCD.h"
#include "PIR_Sensor.h" 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_rom_sys.h"
#include "esp_log.h"

#define LCD_ADDR 0x27

static const char *TAG = "PIR_LCD";

#define LCD_RS (1 << 0)
#define LCD_RW (1 << 1)
#define LCD_EN (1 << 2)
#define LCD_BL (1 << 3)

#define CMD_CLEAR 0x01
#define CMD_ENTRY_MODE 0x04
#define CMD_DISPLAY_CTRL 0x08
#define CMD_FUNCTION_SET 0x20
#define CMD_SET_DDRAM 0x80

static const uint8_t row_offsets[] = {0x00, 0x40};

static esp_err_t pcf_write(i2c_lcd_t *lcd, uint8_t data)
{
    uint8_t byte = data | lcd->backlight;
    return i2c_master_transmit(lcd->dev, &byte, 1, 100);
}

static esp_err_t pulse_enable(i2c_lcd_t *lcd, uint8_t data)
{
    esp_err_t err = pcf_write(lcd, data | LCD_EN);
    if (err != ESP_OK)
        return err;
    esp_rom_delay_us(1);
    err = pcf_write(lcd, data & ~LCD_EN);
    if (err != ESP_OK)
        return err;
    esp_rom_delay_us(50);
    return ESP_OK;
}

static esp_err_t send_nibble(i2c_lcd_t *lcd, uint8_t nibble, uint8_t rs_flag)
{
    return pulse_enable(lcd, (nibble & 0xF0) | rs_flag);
}

static esp_err_t send_byte(i2c_lcd_t *lcd, uint8_t value, uint8_t rs_flag)
{
    esp_err_t err = send_nibble(lcd, value & 0xF0, rs_flag);
    if (err != ESP_OK)
        return err;
    return send_nibble(lcd, (value << 4) & 0xF0, rs_flag);
}

esp_err_t i2c_lcd_init(i2c_lcd_t *lcd, i2c_master_bus_handle_t bus, uint8_t addr)
{
    lcd->backlight = LCD_BL;

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = addr,
        .scl_speed_hz = 100000,
    };
    esp_err_t err = i2c_master_bus_add_device(bus, &dev_cfg, &lcd->dev);
    if (err != ESP_OK)
        return err;

    vTaskDelay(pdMS_TO_TICKS(50));

    send_nibble(lcd, 0x30, 0);
    vTaskDelay(pdMS_TO_TICKS(5));
    send_nibble(lcd, 0x30, 0);
    esp_rom_delay_us(150);
    send_nibble(lcd, 0x30, 0);
    esp_rom_delay_us(150);
    send_nibble(lcd, 0x20, 0);
    esp_rom_delay_us(150);

    send_byte(lcd, CMD_FUNCTION_SET | 0x08, 0);
    send_byte(lcd, CMD_DISPLAY_CTRL | 0x04, 0);
    send_byte(lcd, CMD_CLEAR, 0);
    vTaskDelay(pdMS_TO_TICKS(2));
    send_byte(lcd, CMD_ENTRY_MODE | 0x02, 0);

    return ESP_OK;
}

void i2c_lcd_clear(i2c_lcd_t *lcd)
{
    send_byte(lcd, CMD_CLEAR, 0);
    vTaskDelay(pdMS_TO_TICKS(2));
}

void i2c_lcd_set_cursor(i2c_lcd_t *lcd, uint8_t col, uint8_t row)
{
    if (row > 1)
        row = 1; 
    send_byte(lcd, CMD_SET_DDRAM | (col + row_offsets[row]), 0);
}

void i2c_lcd_print(i2c_lcd_t *lcd, const char *str)
{
    while (*str)
    {
        send_byte(lcd, (uint8_t)*str++, LCD_RS);
    }
}

static void print_line_padded(i2c_lcd_t *lcd, uint8_t row, const char *str)
{
    i2c_lcd_set_cursor(lcd, 0, row);
    char buf[17];
    int len = 0;
    while (str[len] && len < 16)
    {
        buf[len] = str[len];
        len++;
    }
    while (len < 16)
    {
        buf[len++] = ' ';
    }
    buf[16] = '\0';
    i2c_lcd_print(lcd, buf);
}

void pir_lcd_task(void *pvParameters)
{
    i2c_master_bus_handle_t shared_bus = (i2c_master_bus_handle_t)pvParameters;

    i2c_lcd_t lcd;
    esp_err_t err = i2c_lcd_init(&lcd, shared_bus, LCD_ADDR); 
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "LCD init failed: %s", esp_err_to_name(err));
        vTaskDelete(NULL);
        return;
    }

    print_line_padded(&lcd, 0, "No Motion");

    bool last_motion = false;
    PIR_Data_t pir_data;

    while (1)
    {
        if (xQueueReceive(pir_q_hndl, &pir_data, portMAX_DELAY) == pdTRUE)
        {
            if (pir_data.motion != last_motion)
            {
                last_motion = pir_data.motion;
                print_line_padded(&lcd, 0, last_motion ? "Motion Detected" : "No Motion");
                ESP_LOGI(TAG, "PIR: %s", last_motion ? "Motion detected" : "Motion cleared");
            }
        }
    }
}