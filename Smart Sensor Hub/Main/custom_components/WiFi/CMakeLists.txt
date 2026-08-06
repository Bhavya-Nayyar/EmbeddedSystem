idf_component_register(
    SRCS
        "Wifi.c"
        "Http_Client.c"
        "Upload_Task.c"

    INCLUDE_DIRS
        "include"

    REQUIRES
        esp_wifi
        esp_event
        esp_netif
        esp_http_client
        nvs_flash
        DHT11_Sensor
        Thermistor_103
        PIR_Sensor
        Sound_Sensor
        Sensor_Data
)