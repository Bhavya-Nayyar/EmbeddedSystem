idf_component_register(SRCS "Sound_Sensor.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_driver_gpio
                       log
                       SD_Card
                       Sensor_Data)