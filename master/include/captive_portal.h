#ifndef CAPTIVE_PORTAL_H
#define CAPTIVE_PORTAL_H

#include <Arduino.h>

/**
 * Initialize the captive portal (DNS server)
 * Should be called after WiFi AP mode is set up
 */
void captive_portal_init();

/**
 * Handle DNS and HTTP requests for captive portal
 * Should be called in the main loop
 */
void captive_portal_update();

/**
 * Stop the captive portal
 */
void captive_portal_stop();

#endif
