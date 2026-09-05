idf_component_register(SRCS "PIR_LCD.c"
                       INCLUDE_DIRS "include"
                       REQUIRES driver 
                       esp_lcd
                       esp_common 
                       esp_driver_i2c
                       PIR_Sensor
                       SD_Card)