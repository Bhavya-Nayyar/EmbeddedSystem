idf_component_register(SRCS "main.c"
                        INCLUDE_DIRS "."
                        REQUIRES esp_driver_gpio esp_driver_uart fatfs esp_driver_sdspi sdmmc esp_driver_gptimer esp_driver_twai)