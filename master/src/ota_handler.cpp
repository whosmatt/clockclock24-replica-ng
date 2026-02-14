#include "ota_handler.h"
#include "status_led.h"
#include <ElegantOTA.h>

void ota_init(WebServer *server)
{
    // Setup ElegantOTA with callbacks
    ElegantOTA.begin(server);

    // Set callbacks for visual feedback
    ElegantOTA.onStart([]()
    {
        Serial.println("OTA Update Started");
        shutdown(); // Add a fast stop command to the queue to restart in calibrated state
        led_set_status(LED_OTA); // Yellow blinking to indicate OTA in progress
    });

    ElegantOTA.onProgress([](size_t current, size_t final)
    {
        // Called repeatedly during upload
        int percent = (current / (float)final) * 100;
        if (percent % 10 == 0 && percent != 0) // Log every 10%
        {
            Serial.printf("OTA Progress: %d%%\n", percent);
        }
        led_update(); // Update LED for blinking effect
    });

    Serial.println("OTA Update service started");
}

void ota_handle()
{
    ElegantOTA.loop();
}
