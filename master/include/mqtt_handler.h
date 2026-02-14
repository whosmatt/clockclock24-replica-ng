#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <Arduino.h>

/**
 * Initialize MQTT client and connect to broker
 */
void mqtt_init();

/**
 * Handle MQTT client loop and reconnection
 */
void mqtt_handle();

/**
 * Publish current state to MQTT
 */
void mqtt_publish_state();

/**
 * Publish Home Assistant auto-discovery configuration
 */
void mqtt_publish_discovery();

/**
 * Check if MQTT is connected
 */
bool mqtt_is_connected();

#endif
