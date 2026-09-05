#ifndef WIFI_H
#define WIFI_H

void wifi_init_sta(void *pvParameters);

void wifi_disconnect(void);

void wifi_reconnect(void);

#endif