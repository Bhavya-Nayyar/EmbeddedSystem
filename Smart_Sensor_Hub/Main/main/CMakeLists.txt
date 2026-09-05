idf_component_register(SRCS "main.c" 
                        INCLUDE_DIRS "."  
                        REQUIRES esp_driver_gpio 
                        esp_driver_uart 
                        fatfs 
                        esp_driver_sdspi 
                        sdmmc 
                        esp_driver_gptimer 
                        esp_driver_ledc 
                        esp_adc 
                        esp_driver_i2c 
                        esp_timer
                        dht
                        DHT11_Sensor
                        Thermistor_103
                        PIR_Sensor
                        Sound_Sensor
                        Temp_Hum_OLED
                        PIR_LCD
                        Sound_LCD
                        SD_Card
                        WiFi
                        Sensor_Data)