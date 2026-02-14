#include "mqtt_handler.h"
#include "clock_config.h"
#include <WiFi.h>
#include <PubSubClient.h>

// MQTT client
WiFiClient _mqtt_wifi_client;
PubSubClient _mqtt_client(_mqtt_wifi_client);

// MQTT topics
String _mqtt_base_topic;
String _mqtt_state_topic;
String _mqtt_command_topic;

// State tracking
unsigned long _last_reconnect_attempt = 0;
unsigned long _last_state_publish = 0;
const unsigned long RECONNECT_INTERVAL = 5000;      // 5 seconds
const unsigned long STATE_PUBLISH_INTERVAL = 30000; // 30 seconds
bool _state_update_pending = false;

// Forward declarations
void mqtt_callback(char *topic, byte *payload, unsigned int length);
bool mqtt_reconnect();
void publish_discovery_select();
void publish_discovery_switch();
const char *get_mode_string(int mode);

void mqtt_init()
{
    if (!get_mqtt_enabled() || strlen(get_mqtt_broker()) == 0)
    {
        return;
    }

    // Build topic strings based on hostname
    String hostname = String(get_hostname());
    _mqtt_base_topic = "clockclock24/" + hostname;
    _mqtt_state_topic = _mqtt_base_topic + "/state";
    _mqtt_command_topic = _mqtt_base_topic + "/set";

    // Configure MQTT client
    _mqtt_client.setServer(get_mqtt_broker(), get_mqtt_port());
    _mqtt_client.setCallback(mqtt_callback);
    _mqtt_client.setBufferSize(1024);
    _mqtt_client.setKeepAlive(60);
}

bool mqtt_reconnect()
{
    if (!get_mqtt_enabled())
    {
        return false;
    }

    // Disconnect first if connected
    if (_mqtt_client.connected())
    {
        _mqtt_client.disconnect();
    }

    String client_id = "clockclock24-" + String(get_hostname());

    // Connect to broker
    bool connected;
    if (strlen(get_mqtt_username()) > 0)
    {
        connected = _mqtt_client.connect(client_id.c_str(),
                                        get_mqtt_username(),
                                        get_mqtt_password());
    }
    else
    {
        connected = _mqtt_client.connect(client_id.c_str());
    }

    if (!connected)
    {
        return false;
    }

    // Subscribe to command topic
    _mqtt_client.subscribe(_mqtt_command_topic.c_str());

    // Publish discovery and initial state
    mqtt_publish_discovery();
    mqtt_publish_state();

    return true;
}

void mqtt_handle()
{
    if (!get_mqtt_enabled())
    {
        return;
    }

    if (!_mqtt_client.connected())
    {
        unsigned long now = millis();
        if (now - _last_reconnect_attempt > RECONNECT_INTERVAL)
        {
            _last_reconnect_attempt = now;
            if (mqtt_reconnect())
            {
                _last_reconnect_attempt = 0;
            }
        }
    }
    else
    {
        _mqtt_client.loop();

        // Publish pending state updates (from callback processing)
        if (_state_update_pending)
        {
            _state_update_pending = false;
            mqtt_publish_state();
        }

        // Periodic state publish
        unsigned long now = millis();
        if (now - _last_state_publish > STATE_PUBLISH_INTERVAL)
        {
            mqtt_publish_state();
            _last_state_publish = now;
        }
    }
}

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
    String topic_str = String(topic);
    String payload_str = "";

    for (unsigned int i = 0; i < length; i++)
    {
        payload_str += (char)payload[i];
    }

    // Parse JSON command
    if (topic_str == _mqtt_command_topic)
    {
        // Simple JSON parsing - look for "mode":"VALUE" or "power":"VALUE"
        int mode_idx = payload_str.indexOf("\"mode\":\"");
        if (mode_idx >= 0)
        {
            int mode_start = mode_idx + 8; // Length of "mode":"
            int mode_end = payload_str.indexOf("\"", mode_start);
            if (mode_end > mode_start)
            {
                String mode_value = payload_str.substring(mode_start, mode_end);
                if (mode_value == "LAZY")
                {
                    set_clock_animation_mode(LAZY);
                    _state_update_pending = true;
                }
                else if (mode_value == "FUN")
                {
                    set_clock_animation_mode(FUN);
                    _state_update_pending = true;
                }
                else if (mode_value == "WAVES")
                {
                    set_clock_animation_mode(WAVES);
                    _state_update_pending = true;
                }
            }
        }

        int power_idx = payload_str.indexOf("\"power\":\"");
        if (power_idx >= 0)
        {
            int power_start = power_idx + 9; // Length of "power":"
            int power_end = payload_str.indexOf("\"", power_start);
            if (power_end > power_start)
            {
                String power_value = payload_str.substring(power_start, power_end);
                if (power_value == "ON")
                {
                    set_clock_enabled(true);
                    _state_update_pending = true;
                }
                else if (power_value == "OFF")
                {
                    set_clock_enabled(false);
                    _state_update_pending = true;
                }
            }
        }
    }
}

void mqtt_publish_state()
{
    if (!_mqtt_client.connected())
    {
        return;
    }

    // Build JSON state payload
    String state_payload = "{";
    state_payload += "\"mode\":\"" + String(get_mode_string(get_clock_animation_mode())) + "\",";
    state_payload += "\"power\":\"" + String(get_clock_enabled() ? "ON" : "OFF") + "\"";
    state_payload += "}";

    _mqtt_client.publish(_mqtt_state_topic.c_str(), state_payload.c_str(), false);
}

void mqtt_publish_discovery()
{
    if (!_mqtt_client.connected())
    {
        return;
    }

    publish_discovery_select();
    publish_discovery_switch();
}

void publish_discovery_select()
{
    String hostname = String(get_hostname());
    String unique_id = hostname + "_mode";
    String discovery_topic = "homeassistant/select/" + unique_id + "/config";

    // Build JSON discovery payload
    String payload = "{";
    payload += "\"name\":\"Animation\",";
    payload += "\"unique_id\":\"" + unique_id + "\",";
    payload += "\"command_topic\":\"" + _mqtt_command_topic + "\",";
    payload += "\"command_template\":\"{\\\"mode\\\":\\\"{{value}}\\\"}\",";
    payload += "\"state_topic\":\"" + _mqtt_state_topic + "\",";
    payload += "\"value_template\":\"{{value_json.mode}}\",";
    payload += "\"qos\":0,";
    payload += "\"optimistic\":false,";
    payload += "\"options\":[\"LAZY\",\"FUN\",\"WAVES\"],";
    payload += "\"device\":{";
    payload += "\"identifiers\":[\"" + hostname + "\"],";
    payload += "\"name\":\"ClockClock24\",";
    payload += "\"model\":\"ClockClock24 Replica\",";
    payload += "\"manufacturer\":\"Vallasc, whosmatt\"";
    payload += "}";
    payload += "}";

    _mqtt_client.publish(discovery_topic.c_str(), payload.c_str(), true);
}

void publish_discovery_switch()
{
    String hostname = String(get_hostname());
    String unique_id = hostname + "_power";
    String discovery_topic = "homeassistant/switch/" + unique_id + "/config";

    // Build JSON discovery payload
    String payload = "{";
    payload += "\"name\":\"Power\",";
    payload += "\"unique_id\":\"" + unique_id + "\",";
    payload += "\"command_topic\":\"" + _mqtt_command_topic + "\",";
    payload += "\"command_template\":\"{\\\"power\\\":\\\"{{value}}\\\"}\",";
    payload += "\"state_topic\":\"" + _mqtt_state_topic + "\",";
    payload += "\"value_template\":\"{{value_json.power}}\",";
    payload += "\"qos\":0,";
    payload += "\"optimistic\":false,";
    payload += "\"payload_on\":\"ON\",";
    payload += "\"payload_off\":\"OFF\",";
    payload += "\"device\":{";
    payload += "\"identifiers\":[\"" + hostname + "\"],";
    payload += "\"name\":\"ClockClock24\",";
    payload += "\"model\":\"ClockClock24 Replica\",";
    payload += "\"manufacturer\":\"Vallasc, whosmatt\"";
    payload += "}";
    payload += "}";

    _mqtt_client.publish(discovery_topic.c_str(), payload.c_str(), true);
}

const char *get_mode_string(int mode)
{
    switch (mode)
    {
    case LAZY:
        return "LAZY";
    case FUN:
        return "FUN";
    case WAVES:
        return "WAVES";
    default:
        return "LAZY";
    }
}

bool mqtt_is_connected()
{
    return _mqtt_client.connected();
}
