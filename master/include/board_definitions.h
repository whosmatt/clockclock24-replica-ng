#ifndef BOARD_DEFINITIONS_H
#define BOARD_DEFINITIONS_H

// Board-specific pin definitions

#if defined(ARDUINO_LOLIN_C3_MINI)
// LOLIN C3 Mini
#define I2C_SDA 8
#define I2C_SCL 10
#define STATUS_LED_PIN 7
#define STATUS_LED_TYPE (NEO_GRB + NEO_KHZ800)
#define BOARD_NAME "LOLIN C3 Mini"
#elif defined(ARDUINO_LOLIN_S3_MINI)
// LOLIN S3 Mini
#define I2C_SDA 35
#define I2C_SCL 36
#define STATUS_LED_PIN 47
#define STATUS_LED_TYPE (NEO_RGB + NEO_KHZ800)
#define BOARD_NAME "LOLIN S3 Mini"
#endif

#endif
