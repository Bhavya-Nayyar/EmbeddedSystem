#pragma once

/* Call once at startup, after nvs/system init, before anything
   that wants to call oled_show_status(). */
void oled_init_bus(void);

/* Clears the display and draws up to two lines of status text.
   Safe to call before oled_init_bus() succeeds — it will just
   log a warning and skip drawing. */
void oled_show_status(const char *line1, const char *line2);
