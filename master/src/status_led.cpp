#include "status_led.h"
#include <Adafruit_NeoPixel.h>

#define NUM_LEDS 1
#define LED_BRIGHTNESS 3

Adafruit_NeoPixel strip(NUM_LEDS, STATUS_LED_PIN, STATUS_LED_TYPE);

led_status current_status = LED_OFF;
unsigned long last_blink = 0;
bool blink_state = false;

void led_init()
{
    strip.begin();
    strip.setBrightness(LED_BRIGHTNESS);
    strip.show(); // Initialize all pixels to 'off'
}

void led_set_status(led_status status)
{
    current_status = status;

    switch (status)
    {
    case LED_OFF:
        strip.setPixelColor(0, strip.Color(0, 0, 0));
        break;

    case LED_CONNECTING:
        // Will blink in led_update()
        break;

    case LED_CONNECTED:
        strip.setPixelColor(0, strip.Color(0, 255, 0)); // Green
        break;

    case LED_AP_MODE:
        strip.setPixelColor(0, strip.Color(0, 255, 255)); // Cyan
        break;

    case LED_OTA:
        // Will blink in led_update()
        break;

    case LED_ERROR:
        strip.setPixelColor(0, strip.Color(255, 0, 0)); // Red
        break;
    }

    if (status != LED_CONNECTING && status != LED_OTA)
    {
        strip.show();
    }
}

void led_off()
{
    current_status = LED_OFF;
    strip.setPixelColor(0, strip.Color(0, 0, 0));
    strip.show();
}

void led_update()
{
    // Handle blinking for CONNECTING and OTA states
    if (current_status == LED_CONNECTING || current_status == LED_OTA)
    {
        unsigned long now = millis();
        if (now - last_blink >= 400)
        {
            last_blink = now;
            blink_state = !blink_state;

            if (blink_state)
            {
                if (current_status == LED_CONNECTING)
                    strip.setPixelColor(0, strip.Color(0, 0, 255)); // Blue for WiFi
                else if (current_status == LED_OTA)
                    strip.setPixelColor(0, strip.Color(255, 255, 0)); // Yellow for OTA
            }
            else
                strip.setPixelColor(0, strip.Color(0, 0, 0)); // Off

            strip.show();
        }
    }
}
