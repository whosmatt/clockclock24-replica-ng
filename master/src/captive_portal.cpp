#include "captive_portal.h"
#include <DNSServer.h>
#include <WiFi.h>

const byte DNS_PORT = 53;
DNSServer dnsServer;

void captive_portal_init()
{
    // Start DNS server that resolves all queries to the device's IP
    IPAddress apIP = WiFi.softAPIP();

    if (dnsServer.start(DNS_PORT, "*", apIP))
    {
        Serial.println("Captive Portal: DNS server started");
        Serial.printf("All DNS queries will resolve to %s\n", apIP.toString().c_str());
    }
    else
    {
        Serial.println("Captive Portal: Failed to start DNS server");
    }
}

void captive_portal_update()
{
    // Process DNS requests
    dnsServer.processNextRequest();
}

void captive_portal_stop()
{
    dnsServer.stop();
    Serial.println("Captive Portal: DNS server stopped");
}
