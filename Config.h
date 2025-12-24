#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- Pins ---
const int PM_SENSOR_RX_PIN  = 14; // D5 (GPIO 14) -> Sensor TX
const int PM_SENSOR_SET_PIN = 12; // D6 (GPIO 12) -> Sensor SET
const int OLED_SDA_PIN      = 4;  // D2 (GPIO 4)
const int OLED_SCL_PIN      = 5;  // D1 (GPIO 5)
const int DHT_PIN           = 2;  // D4 (GPIO 2)
const int WS2812_PIN        = 15; // D8 (GPIO 15)
const int BUTTON_PIN        = 13; // D7 (GPIO 13)

// --- Timing ---
const unsigned long INITIAL_WARMUP_DURATION = 300000; // 5 mins
const unsigned long PM_WAKE_DURATION        = 32000;  // 32s
const unsigned long PM_SLEEP_DURATION       = 60000; // 60s
const unsigned long STABILITY_THRESHOLD     = 30000;  // 30s stable wait
const unsigned long BLYNK_SEND_INTERVAL     = 60000;  
const unsigned long BLYNK_ICON_KEEP_ALIVE   = 3000;

const char* FIRMWARE_VERSION = "V1.4.0 - Integrated";

#endif
