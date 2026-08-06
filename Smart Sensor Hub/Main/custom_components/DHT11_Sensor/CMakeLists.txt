idf_component_register(SRCS "DHT11_Sensor.c"
                       INCLUDE_DIRS "include"
                       REQUIRES dht
                       esp_driver_gpio
                       log
                       SD_Card
                       Sensor_Data)