#ifndef UPDATE_HANDLER_H
#define UPDATE_HANDLER_H

#include <Arduino.h>
#include <WebServer.h>
#include <Update.h>

/**
 * Initialize update handler
 * Should be called after web server is initialized
 */
void update_init(WebServer* server);

/**
 * Handle update requests
 * Should be called in the main loop
 */
void update_handle();

/**
 * Get the current update status
 * @return status string (JSON)
 */
String update_get_status();

/**
 * Check if an update is in progress
 * @return true if update is running
 */
bool update_in_progress();

/**
 * Cancel the current update
 */
void update_cancel();

/**
 * Schedule a device restart after a delay
 * Stops the clock, sets the LED to error, and restarts via the existing
 * update_handle() reboot path so the restart is always handled cleanly.
 * If an OTA update is in progress, the request is ignored.
 * @param delay_ms  milliseconds to wait before rebooting
 */
void schedule_restart(unsigned long delay_ms);

#endif
