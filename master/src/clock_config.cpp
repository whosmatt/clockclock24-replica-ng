#include "clock_config.h"

// Non volatile preferences
Preferences prefs;

// Internal config state
int _clock_mode;
bool _clock_enabled = true;
bool _sleep_time[7 * 24];
int _clock_timezone;
int _speed_multiplier = 1;

int _wireless_mode;
char _ssid[64];
char _password[64];
char _hostname[64];

// MQTT configuration
char _mqtt_broker[64];
int _mqtt_port = 1883;
char _mqtt_username[64];
char _mqtt_password[64];
bool _mqtt_enabled = false;

void begin_config()
{
  prefs.begin("clockclock24");
  _clock_mode = prefs.getInt("clock_mode", LAZY);
  _clock_enabled = prefs.getBool("clock_enabled", true);
  _wireless_mode = prefs.getInt("wireless_mode", HOTSPOT);
  _clock_timezone = prefs.getInt("clock_timezone", 0);
  _speed_multiplier = prefs.getInt("speed_mult", 1);
  strncpy(_ssid, prefs.getString("ssid", "").c_str(), sizeof(_ssid));
  strncpy(_password, prefs.getString("password", "").c_str(), sizeof(_password));
  strncpy(_hostname, prefs.getString("hostname", "clockclock24").c_str(), sizeof(_hostname));
  strncpy(_mqtt_broker, prefs.getString("mqtt_broker", "").c_str(), sizeof(_mqtt_broker));
  _mqtt_port = prefs.getInt("mqtt_port", 1883);
  strncpy(_mqtt_username, prefs.getString("mqtt_user", "").c_str(), sizeof(_mqtt_username));
  strncpy(_mqtt_password, prefs.getString("mqtt_pass", "").c_str(), sizeof(_mqtt_password));
  _mqtt_enabled = prefs.getBool("mqtt_enabled", false);
  if(prefs.isKey("sleep_time"))
    prefs.getBytes("sleep_time", _sleep_time, sizeof(_sleep_time));
  else
    memset(_sleep_time, 0, sizeof(_sleep_time));
}

void end_config()
{
  prefs.end();
}

void clear_config()
{
  prefs.clear();
  _clock_mode = LAZY;
  _clock_enabled = true;
  _wireless_mode = HOTSPOT;
  _speed_multiplier = 1;
  strncpy(_ssid, "", sizeof(_ssid));
  strncpy(_password, "", sizeof(_password));
  strncpy(_hostname, "clockclock24", sizeof(_hostname));
  strncpy(_mqtt_broker, "", sizeof(_mqtt_broker));
  _mqtt_port = 1883;
  strncpy(_mqtt_username, "", sizeof(_mqtt_username));
  strncpy(_mqtt_password, "", sizeof(_mqtt_password));
  _mqtt_enabled = false;
  memset(_sleep_time, 0, sizeof(_sleep_time));
}

int get_clock_mode()
{
  // Return OFF if clock is disabled, otherwise return the animation mode
  return _clock_enabled ? _clock_mode : OFF;
}

int get_clock_animation_mode()
{
  return _clock_mode;
}

bool get_clock_enabled()
{
  return _clock_enabled;
}

bool get_sleep_time(int day, int hour)
{
  return _sleep_time[(day * 24) + (hour % 24)];
}

int get_connection_mode()
{
  return _wireless_mode;
}

int get_timezone()
{
  return _clock_timezone;
}

char *get_ssid()
{
  return _ssid;
}

char *get_password()
{
  return _password;
}

char *get_hostname()
{
  return _hostname;
}

char *get_mqtt_broker()
{
  return _mqtt_broker;
}

int get_mqtt_port()
{
  return _mqtt_port;
}

char *get_mqtt_username()
{
  return _mqtt_username;
}

char *get_mqtt_password()
{
  return _mqtt_password;
}

bool get_mqtt_enabled()
{
  return _mqtt_enabled;
}

int get_speed_multiplier()
{
  return _speed_multiplier;
}

void set_clock_mode(int value)
{
  // Handle OFF as a special case - disable the clock
  if (value == OFF) {
    _clock_enabled = false;
    prefs.putBool("clock_enabled", false);
  } else {
    _clock_mode = value;
    _clock_enabled = true;
    prefs.putInt("clock_mode", value);
    prefs.putBool("clock_enabled", true);
  }
}

void set_clock_mode_temp(int value)
{
  // Handle OFF as a special case - disable the clock temporarily
  if (value == OFF) {
    _clock_enabled = false;
  } else {
    _clock_mode = value;
    _clock_enabled = true;
  }
}

void set_clock_animation_mode(int value)
{
  if (value >= LAZY && value <= WAVES) {
    _clock_mode = value;
    prefs.putInt("clock_mode", value);
  }
}

void set_clock_enabled(bool value)
{
  _clock_enabled = value;
  prefs.putBool("clock_enabled", value);
}

void set_sleep_time(int day, int hour, bool value)
{
  _sleep_time[(day * 24) + (hour % 24)] = value;
}

void save_sleep_time()
{
  prefs.putBytes("sleep_time", _sleep_time, sizeof(_sleep_time));
}

void set_connection_mode(int value)
{
  _wireless_mode = value;
  prefs.putInt("wireless_mode", value);
}

void set_timezone(int value)
{
  _clock_timezone = value;
  prefs.putInt("clock_timezone", value);
}

void set_ssid(const char *value)
{
  strncpy(_ssid, value, sizeof(_ssid));
  prefs.putString("ssid", value);
}

void set_password(const char *value)
{
  strncpy(_password, value, sizeof(_password));
  prefs.putString("password", value);
}

void set_hostname(const char *value)
{
  // Use default if empty
  const char *hostname = (value && strlen(value) > 0) ? value : "clockclock24";
  strncpy(_hostname, hostname, sizeof(_hostname));
  prefs.putString("hostname", hostname);
}

void set_mqtt_broker(const char *value)
{
  strncpy(_mqtt_broker, value, sizeof(_mqtt_broker));
  prefs.putString("mqtt_broker", value);
}

void set_mqtt_port(int value)
{
  _mqtt_port = value;
  prefs.putInt("mqtt_port", value);
}

void set_mqtt_username(const char *value)
{
  strncpy(_mqtt_username, value, sizeof(_mqtt_username));
  prefs.putString("mqtt_user", value);
}

void set_mqtt_password(const char *value)
{
  strncpy(_mqtt_password, value, sizeof(_mqtt_password));
  prefs.putString("mqtt_pass", value);
}

void set_mqtt_enabled(bool value)
{
  _mqtt_enabled = value;
  prefs.putBool("mqtt_enabled", value);
}

void set_speed_multiplier(int value)
{
  if(value >= 1)
  {
    _speed_multiplier = value;
    prefs.putInt("speed_mult", value);
  }
}