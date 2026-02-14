#ifndef CLOCK_CONFIG_H
#define CLOCK_CONFIG_H

#include <Preferences.h>

/** 
 * Clock connection's modes
 */
enum wireless_modes
{
  HOTSPOT,
  EXT_CONN
};

/** 
 * Clock animation's modes
 */
enum clock_modes
{
  LAZY,
  FUN,
  WAVES
};

// Virtual mode for backward compatibility
#define OFF 255

/**
 * Load configuration from the EEPROM
 */
void begin_config();

/**
 * Clear EEPROM configuration
 */
void clear_config();

/**
 * Closes the preferencies object
 */
void end_config();

/**
 * Get current clock mode (returns OFF if clock is disabled)
 */
int get_clock_mode();

/**
 * Get current clock animation mode (ignores power state)
 */
int get_clock_animation_mode();

/**
 * Get clock enabled state
 */
bool get_clock_enabled();

/**
 * Gets current sleep time at a given day and hour
 * @param day   day of the week
 * @param hour  hour of the day
 */
bool get_sleep_time(int day, int hour);

/**
 * Gets current connection mode
 */
int get_connection_mode();

/**
 * Gets active (runtime) connection mode
 */
int get_active_connection_mode();

/**
 * Gets current time zone based on UTC offset
 */
int get_timezone();

/**
 * Gets current SSID
 */
char *get_ssid();

/**
 * Gets current password
 */
char *get_password();

/**
 * Gets current hostname
 */
char *get_hostname();

/**
 * Gets MQTT broker address
 */
char *get_mqtt_broker();

/**
 * Gets MQTT broker port
 */
int get_mqtt_port();

/**
 * Gets MQTT username
 */
char *get_mqtt_username();

/**
 * Gets MQTT password
 */
char *get_mqtt_password();

/**
 * Gets MQTT enabled state
 */
bool get_mqtt_enabled();

/**
 * Gets current speed multiplier
 * @return speed multiplier (positive integer)
 */
int get_speed_multiplier();

/**
 * Sets clock mode
 * @param value   mode value of type clock_modes or OFF
 */
void set_clock_mode(int value);

/**
 * Sets clock mode temporarily (non-persistent)
 * @param value   mode value of type clock_modes or OFF
 */
void set_clock_mode_temp(int value);

/**
 * Sets clock animation mode (0-2: LAZY, FUN, WAVES)
 * @param value   mode value of type clock_modes
 */
void set_clock_animation_mode(int value);

/**
 * Sets clock enabled state
 * @param value   true to enable, false to disable
 */
void set_clock_enabled(bool value);

/**
 *  Sets current sleep time at a given day and hour
 * @param day   day of the week
 * @param hour  hour of the day
 * @param value true if clock is  disabled, false otherwise
 */
void set_sleep_time(int day, int hour, bool value);

/**
 *  Saves sleep time array on EEPROM
 */
void save_sleep_time();

/**
 *  Sets connection mode
 * @param value   mode value of type wireless_modes
 */
void set_connection_mode(int value);

/**
 * Sets active connection mode (runtime only, not saved)
 * @param value  new active connection mode
 */
void set_active_connection_mode(int value);

/**
 *  Sets the time zone
 * @param value   time zone based on UTC offset
 */
void set_timezone(int value);

/**
 *  Sets SSID value
 * @param value   SSID string
 */
void set_ssid(const char *value);

/**
 *  Sets password value
 * @param value   password string
 */
void set_password(const char *value);

/**
 *  Sets hostname value
 * @param value   hostname string (defaults to "clockclock24" if empty)
 */
void set_hostname(const char *value);

/**
 *  Sets MQTT broker address
 * @param value   broker IP or hostname
 */
void set_mqtt_broker(const char *value);

/**
 *  Sets MQTT broker port
 * @param value   broker port (default 1883)
 */
void set_mqtt_port(int value);

/**
 *  Sets MQTT username
 * @param value   MQTT username
 */
void set_mqtt_username(const char *value);

/**
 *  Sets MQTT password
 * @param value   MQTT password
 */
void set_mqtt_password(const char *value);

/**
 *  Sets MQTT enabled state
 * @param value   true to enable MQTT, false to disable
 */
void set_mqtt_enabled(bool value);

/**
 * Sets speed multiplier
 * @param value   speed multiplier (positive integer)
 */
void set_speed_multiplier(int value);

#endif