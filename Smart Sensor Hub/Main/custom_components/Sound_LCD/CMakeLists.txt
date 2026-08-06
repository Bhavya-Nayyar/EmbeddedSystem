idf_component_register(SRCS "Sound_LCD.c"
                       INCLUDE_DIRS "include"
                       REQUIRES driver 
                       esp_lcd
                       esp_common 
                       esp_driver_i2c
                       Sound_Sensor
                       SD_Card)