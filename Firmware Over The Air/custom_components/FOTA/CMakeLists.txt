idf_component_register(SRCS "FOTA.c"
                        INCLUDE_DIRS "include"
                        REQUIRES esp_http_client app_update esp_wifi log OLED)
