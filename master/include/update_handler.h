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

#endif
