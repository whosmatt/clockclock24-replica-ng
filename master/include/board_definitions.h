#ifndef BOARD_DEFINITIONS_H
#define BOARD_DEFINITIONS_H

// Board-specific pin definitions

#if defined(CONFIG_IDF_TARGET_ESP32C3)
// LOLIN C3 Mini
#define I2C_SDA 8
#define I2C_SCL 10
#define BOARD_NAME "LOLIN C3 Mini"
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
// LOLIN S3 Mini
#define I2C_SDA 35
#define I2C_SCL 36
#define BOARD_NAME "LOLIN S3 Mini"
#endif

#endif
