#ifndef PIR_LCD_H
#define PIR_LCD_H

#include "driver/i2c_master.h"
#include "esp_err.h"

typedef struct
{
    i2c_master_dev_handle_t dev;
    uint8_t backlight;
} i2c_lcd_t;

esp_err_t i2c_lcd_init(i2c_lcd_t *lcd, i2c_master_bus_handle_t bus, uint8_t addr);

void i2c_lcd_clear(i2c_lcd_t *lcd);

void i2c_lcd_set_cursor(i2c_lcd_t *lcd, uint8_t col, uint8_t row);

void i2c_lcd_print(i2c_lcd_t *lcd, const char *str);

void pir_lcd_task(void *pvParameters);

#endif