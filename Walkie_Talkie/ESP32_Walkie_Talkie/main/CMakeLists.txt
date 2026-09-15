idf_component_register(SRCS "main.c"
                        INCLUDE_DIRS "."
                        REQUIRES nopnop2002__mirf esp_driver_spi esp_driver_gpio
                       )