#ifndef SOUND_LCD_H
#define SOUND_LCD_H

#include "driver/i2c_master.h"
#include "esp_err.h"

typedef struct
{
    i2c_master_dev_handle_t dev;
    uint8_t backlight;
} sound_lcd_t;

void sound_lcd_task(void *pvParameters);

#endif