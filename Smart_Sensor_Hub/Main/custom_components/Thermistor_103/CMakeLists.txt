idf_component_register(SRCS "Thermistor_103.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_driver_gpio
                       log
                       esp_adc
                       SD_Card
                       Sensor_Data)