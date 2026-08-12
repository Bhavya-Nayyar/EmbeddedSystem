#pragma once

/* Must be called early in app_main, before anything that could
   crash. Confirms the currently running image if it was just
   OTA-flashed and is pending verification, so the bootloader
   doesn't roll it back on next boot. No-op if rollback support
   isn't enabled in menuconfig or the image isn't pending. */
void fota_validate_running_app(void);

/* Blocking: connects to CONFIG_OTA_FIRMWARE_URL, downloads and
   flashes the image to the inactive OTA partition, verifies
   size, sets the new boot partition, and restarts. Returns only
   on failure (logs the reason and returns without restarting). */
void fota_check_and_update(void);
