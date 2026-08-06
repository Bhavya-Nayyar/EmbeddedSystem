#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include "stdbool.h"

void http_send_data(float temperature,
                    float humidity,
                    float thermistor,
                    bool pir_motion,
                    bool sound_detected);

#endif