#pragma once

/* Starts a FreeRTOS task that blinks the LED on GPIO22 at a rate
   determined by fw_version (see blink_ms_by_version[] in LED.c). */
void led_start_task(int fw_version);
