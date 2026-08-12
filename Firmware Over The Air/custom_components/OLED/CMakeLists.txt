idf_component_register(SRCS "OLED.c"
                        INCLUDE_DIRS "include"
                        REQUIRES driver log k0i05__esp_ssd1306 esp_driver_i2c esp_ssd1306)
