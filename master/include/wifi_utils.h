#ifndef WIFI_UTILS_H
#define WIFI_UTILS_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "status_led.h"

const int max_wifi_retries = 50;

/**
 * Connects to a wifi network
 * @param ssid      access point SSID
 * @param password  access point password
 * @param mdns      mdns host name
 * @return true if successful, false otherwise
*/
bool wifi_connect(const char *ssid, const char *password, const char *mdns)
{
  Serial.printf("\nConnecting to %s\n", ssid);

  led_set_status(LED_CONNECTING);
  WiFi.setHostname(mdns); // Set hostname before mode, see https://github.com/espressif/arduino-esp32/issues/6278
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int i = max_wifi_retries;
  while (WiFi.status() != WL_CONNECTED && i-- > 0)
  {
    delay(200);
    led_update();
    Serial.print(".");
    delay(200);
    led_update();
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.printf("\nWiFi not connected: max wifi retries reached\n");
    led_set_status(LED_ERROR);
    return false;
  }
  if (!MDNS.begin(mdns)) 
  { // Start the mDNS responder
    Serial.println("Error setting up MDNS responder!");
  } 
  else 
  {
    MDNS.addService("http", "tcp", 80);
  }
  Serial.println("WiFi connected");
  Serial.printf("mDNS started: http://%s.local\n", mdns);
  Serial.println("IP address: " + WiFi.localIP().toString());
  led_set_status(LED_CONNECTED);
  return true;
}

/**
 * Creates a wifi network
 * @param ssid    access point SSID
 * @param mdns    mdns host name
 * @return true if successful, false otherwise
*/
bool wifi_create_AP(const char *ssid, const char *mdns)
{
  // Set static IP
  IPAddress AP_LOCAL_IP(192, 168, 1, 10);
  IPAddress AP_GATEWAY_IP(192, 168, 1, 254);
  IPAddress AP_NETWORK_MASK(255, 255, 255, 0);
  Serial.println("\nCreating access point");
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAPConfig(AP_LOCAL_IP, AP_GATEWAY_IP, AP_NETWORK_MASK))
  {
    Serial.println("AP Config Failed");
    led_set_status(LED_ERROR);
    return false;
  }
  if (!WiFi.softAP(ssid, NULL))
  {
    Serial.println("AP Start Failed");
    led_set_status(LED_ERROR);
    return false;
  }
  IPAddress IP = WiFi.softAPIP();
  if (!MDNS.begin(mdns)) 
  { // Start the mDNS responder
    Serial.println("Error setting up MDNS responder!");
  }
  else 
  {
    MDNS.addService("http", "tcp", 80);
  }
  Serial.printf("mDNS started: http://%s.local\n", mdns);
  Serial.println("IP address: " + AP_LOCAL_IP.toString());
  Serial.println("Gateway IP: " + AP_GATEWAY_IP.toString());
  led_set_status(LED_AP_MODE);
  return true;
}

/**
 * Check wifi connection
 * @return true if wifi is connected, false otherwise
*/
bool is_connected()
{
  return WiFi.status() == WL_CONNECTED;
}

#endif