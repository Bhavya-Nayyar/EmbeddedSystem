idf_component_register(SRCS "SD_Card.c"
                       INCLUDE_DIRS "include" "."
                        REQUIRES fatfs
                        driver 
                        esp_driver_sdspi
                        sdmmc
                        esp_timer
                        )