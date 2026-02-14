#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <Arduino.h>
#include "board_definitions.h"

/**
 * LED status states
 */
enum led_status
{
  LED_OFF,              // Off
  LED_CONNECTING,       // Connecting to WiFi (blinking blue)
  LED_CONNECTED,        // Connected successfully (green)
  LED_AP_MODE,          // Access Point mode (cyan)
  LED_OTA,              // OTA update in progress (blinking yellow)
  LED_ERROR             // Error state (red)
};

/**
 * Initialize the status LED
 */
void led_init();

/**
 * Set LED to a specific status
 * @param status  The LED status to set
 */
void led_set_status(led_status status);

/**
 * Turn off the LED
 */
void led_off();

/**
 * Update LED (call in loop for blinking states)
 */
void led_update();

#endif
