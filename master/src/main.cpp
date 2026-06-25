#include <Arduino.h>
#include <Wire.h>
#include <TimeLib.h>

#include "board_definitions.h"
#include "i2c.h"
#include "clock_state.h"
#include "clock_manager.h"
#include "digit.h"
#include "wifi_utils.h"
#include "web_server.h"
#include "clock_config.h"
#include "ntp.h"
#include "status_led.h"
#include "captive_portal.h"
#include "mqtt_handler.h"
#include "update_handler.h"

int last_hour = -1;
int last_minute = -1;
bool is_stopped = false;
const unsigned long SCHEDULED_RESTART_DELAY_MS = 3000;
static int last_restart_day = -1;

/**
 * Sets clock to the current time
*/
void set_time();

/**
 * Sets clock time using lazy animation
*/
void set_lazy();

/**
 * Sets clock time using fun animation
*/
void set_fun();

/**
 * Sets clock time using waves animation
*/
void set_waves();

/**
 * Sets clock time using propeller animation
*/
void set_propeller();

/**
 * Sets clock time using arrow animation
*/
void set_arrow();

/**
 * Sets clock time using ripple animation
*/
void set_ripple();

/**
 * Sets clock time using bubble animation
*/
void set_bubble();

/**
 * Sets clock time using gear animation
*/
void set_gear();

/**
 * Sets clock time using diagonal animation
*/
void set_diagonal();

/**
 * Sets clock time using cascade animation
*/
void set_cascade();

/**
 * Cycles through the animations based on the current minute
*/
void set_cycle();

/**
 * Runs the animation for the given clock mode
 * @param mode    clock mode
*/
void dispatch_animation(int mode);

/**
 * Sets clock to stop state
*/
void stop();

/**
 * Quickly stops the clock with fixed speed for OTA
*/
void shutdown();

/**
 * Custom delay to update web clients
 * @param value   time in milliseconds
*/
void _delay(int value);

void setup() {
  Serial.begin(115200);
  Serial.printf("\nclockclock24 replica by Vallasc - %s\n", BOARD_TARGET);
  delay(3000);
  
  // Initialize status LED
  led_init();
  
  // Load configuration from EEPROM
  begin_config();

  Wire.begin(I2C_SDA, I2C_SCL);

  if(get_connection_mode() == HOTSPOT)
    wifi_create_AP("ClockClock 24", get_hostname());
  else if( !wifi_connect(get_ssid(), get_password(), get_hostname()) )
  {
    // Fall back to hotspot temporarily without changing stored configuration
    set_active_connection_mode(HOTSPOT);
    wifi_create_AP("ClockClock 24", get_hostname());
  }

  if(get_active_connection_mode() == EXT_CONN)
  {
    // Initialize NTP
    begin_NTP();
    setSyncProvider(get_NTP_time);
    // Sync every 30 minutes
    setSyncInterval(60 * 30);
    
    // Initialize MQTT (only in external connection mode)
    mqtt_init();
  }

  // Starts web server
  server_start();
}

void loop() {
  // Daily restart at configured hour
  if (!update_in_progress() && get_daily_restart_enabled())
  {
    int current_hour = hour();
    int current_day = day();
    if (current_hour == get_daily_restart_hour() && current_day != last_restart_day)
    {
      last_restart_day = current_day;
      Serial.printf("Daily restart at %d:00\n", current_hour);
      schedule_restart(SCHEDULED_RESTART_DELAY_MS);
    }
  }

  if (!update_in_progress())
  {
    if (get_active_connection_mode() == EXT_CONN)
    {
      if (wifi_sta_watchdog_should_restart())
      {
        Serial.println("Scheduling restart from STA watchdog");
        schedule_restart(SCHEDULED_RESTART_DELAY_MS);
      }
    }
    else if (get_active_connection_mode() == HOTSPOT)
    {
      if (wifi_ap_watchdog_should_restart())
      {
        Serial.println("Scheduling restart from AP watchdog");
        schedule_restart(SCHEDULED_RESTART_DELAY_MS);
      }
    }
    else
    {
      led_set_status(LED_ERROR);
    }
  }

  led_update();
  
  if(get_active_connection_mode() == HOTSPOT)
    captive_portal_update();

  if(get_active_connection_mode() == HOTSPOT && is_time_changed_browser())
  {
    t_browser_time browser_time = get_browser_time();
    setTime(browser_time.hour, 
      browser_time.minute, 
      browser_time.second, 
      browser_time.day, 
      browser_time.month,  
      browser_time.year);
  }

  if(get_active_connection_mode() == EXT_CONN && get_timezone() != get_ntp_timezone())
  {
    set_ntp_timezone(get_timezone());
    setSyncProvider(get_NTP_time);
  }

  get_clock_mode() != OFF ? set_time() : stop();

  handle_webclient();
  
  // Handle MQTT
  if(get_active_connection_mode() == EXT_CONN)
    mqtt_handle();
}

void set_time()
{
  int day_week = (weekday() + 5) % 7;
  if(get_sleep_time(day_week, hour()))
    stop();
  else if(hour() != last_hour || minute() != last_minute)
  {
    is_stopped = false;
    last_hour = hour();
    last_minute = minute();
    dispatch_animation(get_clock_mode());
  }
}

void dispatch_animation(int mode)
{
  switch(mode)
  {
    case LAZY:
      set_lazy();
      break;
    case FUN:
      set_fun();
      break;
    case WAVES:
      set_waves();
      break;
    case PROPELLER:
      set_propeller();
      break;
    case ARROW:
      set_arrow();
      break;
    case RIPPLE:
      set_ripple();
      break;
    case BUBBLE:
      set_bubble();
      break;
    case GEAR:
      set_gear();
      break;
    case DIAGONAL:
      set_diagonal();
      break;
    case CASCADE:
      set_cascade();
      break;
    case CYCLE:
      set_cycle();
      break;
  }
}

void set_lazy()
{
  set_speed(200 * get_speed_multiplier());
  set_acceleration(100 * get_speed_multiplier());
  set_direction(MIN_DISTANCE);
  set_clock_time(last_hour, last_minute);
}

void set_fun()
{
  set_speed(400 * get_speed_multiplier());
  set_acceleration(150 * get_speed_multiplier());
  set_direction(CLOCKWISE2);
  set_clock_time(last_hour, last_minute);
}

void set_waves()
{
  set_speed(800 * get_speed_multiplier());
  set_acceleration(150 * get_speed_multiplier());
  set_direction(MIN_DISTANCE);
  set_clock(d_IIII);
  _delay(4000 +(9000 - 4000) / sqrt(get_speed_multiplier()));
  set_speed(400 * get_speed_multiplier());
  set_acceleration(100 * get_speed_multiplier());
  set_direction(CLOCKWISE2);
  t_full_clock clock = get_clock_state_from_time(last_hour, last_minute);
  for (int i = 0; i <8; i++)
  {
    set_half_digit(i, clock.digit[i/2].halfs[i%2]);
    delay(200 + (400 - 200) / sqrt(get_speed_multiplier()));
  }
}

void set_propeller()
{
  set_speed(450 * get_speed_multiplier());
  set_acceleration(150 * get_speed_multiplier());
  set_direction(CLOCKWISE3);
  t_full_clock clock = get_clock_state_from_time(last_hour, last_minute);
  for (int i = 0; i < 8; i++)
  {
    t_half_digit hd = get_full_half_digit(clock.digit[i/2].halfs[i%2]);
    for (int j = 0; j < 3; j++)
    {
      hd.clocks[j].mode_h = CLOCKWISE3;
      hd.clocks[j].mode_m = COUNTERCLOCKWISE3;
    }
    set_half_digit_full(i, hd);
  }
}

void set_arrow()
{
  set_speed(600 * get_speed_multiplier());
  set_acceleration(150 * get_speed_multiplier());
  set_direction(MIN_DISTANCE);
  set_clock(d_joint);
  _delay(5000 + (9000 - 4000) / sqrt(get_speed_multiplier()));
  set_speed(450 * get_speed_multiplier());
  set_acceleration(150 * get_speed_multiplier());
  set_direction(CLOCKWISE3);

  t_full_clock target = get_clock_state_from_time(last_hour, last_minute);

  // Diagonal wavefront: project each clock onto (2, -7), stagger by projection
  const int MIN_PROJ = -14;
  const int MAX_PROJ = 14;
  for (int proj = MIN_PROJ; proj <= MAX_PROJ; proj++)
  {
    bool group_has_clock = false;
    for (int hd = 0; hd < 8; hd++)
    {
      t_half_digitl lite = target.digit[hd / 2].halfs[hd % 2];
      for (int p = 0; p < 3; p++)
      {
        if (2 * hd - 7 * p == proj)
        {
          set_single_clock_full(hd, p, lite, CLOCKWISE3, COUNTERCLOCKWISE3);
          group_has_clock = true;
        }
      }
    }
    if (group_has_clock && proj < MAX_PROJ)
      delay(25);
  }
}

void set_ripple()
{
  set_speed(600 * get_speed_multiplier());
  set_acceleration(150 * get_speed_multiplier());
  set_direction(MIN_DISTANCE);
  set_clock(d_WAVE);
  _delay(5000 + (9000 - 4000) / sqrt(get_speed_multiplier()));
  set_speed(600 * get_speed_multiplier());
  set_acceleration(500 * get_speed_multiplier());
  set_direction(CLOCKWISE3);

  t_full_clock target = get_clock_state_from_time(last_hour, last_minute);

  // Stagger by Manhattan distance from the grid centre (col 3.5, row 1)
  const int MAX_DIST = 4;
  for (int d = 0; d <= MAX_DIST; d++)
  {
    for (int hd = 0; hd < 8; hd++)
    {
      // Mirror rotation direction for the right half
      int mode_h = (hd < 4) ? CLOCKWISE3 : COUNTERCLOCKWISE3;
      int mode_m = (hd < 4) ? COUNTERCLOCKWISE3 : CLOCKWISE3;
      t_half_digitl lite = target.digit[hd / 2].halfs[hd % 2];
      for (int p = 0; p < 3; p++)
      {
        int dist = (int)(fabsf((float)hd - 3.5f) + fabsf((float)p - 1.0f));
        if (dist == d)
          set_single_clock_full(hd, p, lite, mode_h, mode_m);
      }
    }
    if (d < MAX_DIST)
      delay(250);
  }
}

void set_bubble()
{
  set_speed(800 * get_speed_multiplier());
  set_acceleration(150 * get_speed_multiplier());
  set_direction(MIN_DISTANCE);
  set_clock(d_bubble);
  _delay(4000 + (9000 - 4000) / sqrt(get_speed_multiplier()));
  set_speed(600 * get_speed_multiplier());
  set_acceleration(150 * get_speed_multiplier());
  set_direction(CLOCKWISE3);
  t_full_clock clock = get_clock_state_from_time(last_hour, last_minute);
  for (int i = 0; i < 8; i++)
  {
    t_half_digit hd = get_full_half_digit(clock.digit[i/2].halfs[i%2]);
    for (int j = 0; j < 3; j++)
    {
      // Checkerboard: alternate spin direction by (col + row) parity
      bool hour_cw = ((i + j) % 2 == 0);
      if (i % 2 == 0)
        hour_cw = !hour_cw;
      if (hour_cw)
      {
        hd.clocks[j].mode_h = CLOCKWISE3;
        hd.clocks[j].mode_m = COUNTERCLOCKWISE3;
      }
      else
      {
        hd.clocks[j].mode_h = COUNTERCLOCKWISE3;
        hd.clocks[j].mode_m = CLOCKWISE3;
      }
    }
    set_half_digit_full(i, hd);
  }
}

void set_gear()
{
  set_speed(600 * get_speed_multiplier());
  set_acceleration(150 * get_speed_multiplier());
  set_direction(MIN_DISTANCE);
  set_clock(d_CENT);
  _delay(5000 + (9000 - 4000) / sqrt(get_speed_multiplier()));
  set_speed(600 * get_speed_multiplier());
  set_acceleration(150 * get_speed_multiplier());
  set_direction(CLOCKWISE3);

  t_full_clock target = get_clock_state_from_time(last_hour, last_minute);

  // Stagger outwards from the centre in a square (Chebyshev) radius
  const int MAX_GROUP = 3;
  for (int g = 0; g <= MAX_GROUP; g++)
  {
    for (int hd = 0; hd < 8; hd++)
    {
      t_half_digitl lite = target.digit[hd / 2].halfs[hd % 2];
      for (int p = 0; p < 3; p++)
      {
        int group = max(min(abs(hd - 3), abs(hd - 4)), abs(p - 1));
        if (group == g)
          set_single_clock_full(hd, p, lite, CLOCKWISE3, CLOCKWISE3);
      }
    }
    if (g < MAX_GROUP)
      delay(300);
  }
}

void set_diagonal()
{
  set_speed(800 * get_speed_multiplier());
  set_acceleration(150 * get_speed_multiplier());
  set_direction(MIN_DISTANCE);
  set_clock(d_diagonal);
  _delay(4000 + (9000 - 4000) / sqrt(get_speed_multiplier()));
  set_speed(400 * get_speed_multiplier());
  set_acceleration(100 * get_speed_multiplier());
  set_direction(CLOCKWISE2);
  t_full_clock clock = get_clock_state_from_time(last_hour, last_minute);
  for (int i = 0; i < 8; i++)
  {
    set_half_digit(i, clock.digit[i/2].halfs[i%2]);
    delay(200 + (400 - 200) / sqrt(get_speed_multiplier()));
  }
}

void set_cascade()
{
  set_speed(800 * get_speed_multiplier());
  set_acceleration(150 * get_speed_multiplier());
  set_direction(MIN_DISTANCE);
  set_clock(d_stop);
  _delay(4000 + (9000 - 4000) / sqrt(get_speed_multiplier()));
  set_speed(300 * get_speed_multiplier());
  set_acceleration(150);
  set_direction(COUNTERCLOCKWISE3);

  t_full_clock clock = get_clock_state_from_time(last_hour, last_minute);

  // Column-by-column reveal, minute hand spins twice as far at twice the speed
  for (int i = 0; i < 8; i++)
  {
    t_half_digit hd = get_full_half_digit(clock.digit[i/2].halfs[i%2]);
    for (int j = 0; j < 3; j++)
    {
      hd.clocks[j].mode_h = COUNTERCLOCKWISE2;
      hd.clocks[j].mode_m = COUNTERCLOCKWISE3;
      hd.clocks[j].speed_m = 600 * get_speed_multiplier();
    }
    set_half_digit_full(i, hd);
    if (i < 7)
      delay(400);
  }
}

void set_cycle()
{
  // Derive the animation from the minute so the web preview stays in sync
  static const int cycle_order[] = {
    FUN, WAVES, ARROW, RIPPLE, BUBBLE, PROPELLER, DIAGONAL, GEAR, CASCADE
  };
  static const int cycle_count = sizeof(cycle_order) / sizeof(cycle_order[0]);
  int minutes_today = last_hour * 60 + last_minute;
  dispatch_animation(cycle_order[minutes_today % cycle_count]);
}

void stop()
{
  if(!is_stopped)
  {
    is_stopped = true;
    last_hour = -1;
    last_minute = -1;
    set_direction(MIN_DISTANCE);
    set_speed(200 * get_speed_multiplier());
    set_acceleration(100 * get_speed_multiplier());
    set_clock(d_stop);
  }
}

void shutdown()
{
  set_clock_mode_temp(OFF);
  if(!is_stopped)
  {
    is_stopped = true;
    last_hour = -1;
    last_minute = -1;
    set_direction(MIN_DISTANCE);
    set_speed(200 * 100);
    set_acceleration(100 * 100);
    set_clock(d_stop);
  }
}

void _delay(int value)
{
  for (int i = 0; i <value/100; i++)
  {
    handle_webclient();
    delay(value/100);
  }
}