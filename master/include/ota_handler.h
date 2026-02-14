#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H

#include <Arduino.h>
#include <WebServer.h>

/**
 * Initialize OTA update handler
 * Should be called after web server is initialized
 */
void ota_init(WebServer* server);

/**
 * Handle OTA update requests
 * Should be called in the main loop
 */
void ota_handle();

#endif
