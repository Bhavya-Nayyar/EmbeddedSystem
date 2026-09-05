idf_component_register(SRCS "Temp_Hum_OLED.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_driver_gpio
                       driver 
                       esp_lcd
                       PRIV_REQUIRES DHT11_Sensor Thermistor_103 k0i05__esp_ssd1306 SD_Card)