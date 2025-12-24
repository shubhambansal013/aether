#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- Pins (NodeMCU ESP8266) ---
// PM Sensor (PMS5003)
const int PM_SENSOR_RX_PIN  = 14; // D5 (GPIO 14) -> Sensor TX
const int PM_SENSOR_SET_PIN = 12; // D6 (GPIO 12) -> Sensor SET

// I2C OLED (SSD1306)
const int OLED_SDA_PIN      = 4;  // D2 (GPIO 4)
const int OLED_SCL_PIN      = 5;  // D1 (GPIO 5)

// Secondary Sensors & UI
const int DHT_PIN           = 2;  // D4 (GPIO 2)
const int WS2812_PIN        = 15; // D8 (GPIO 15)
const int BUTTON_PIN        = 13; // D7 (GPIO 13)

// --- Default Settings ---
// User choice for startup: 0 = AUTO, 1 = ACTIVE, 2 = PASSIVE
const int DEFAULT_MODE_SETTING = 0; 
const uint8_t DEFAULT_OLED_BRIGHTNESS = 150; // 0 to 255

// --- Timing Constants (in Milliseconds) ---
// Initial 5-min warmup for Auto Mode
const unsigned long INITIAL_WARMUP_DURATION = 300000; 

// Passive Cycle: 32s fan on, 60s fan off
const unsigned long PM_WAKE_DURATION        = 32000;  
const unsigned long PM_SLEEP_DURATION       = 60000; 

// How long to wait for air to stabilize before taking a reading
const unsigned long STABILITY_THRESHOLD      = 30000;  

// Cloud & UI Timers
const unsigned long BLYNK_SEND_INTERVAL      = 60000;  
const unsigned long BLYNK_ICON_KEEP_ALIVE    = 3000;   // 3 seconds

// --- Meta ---
const char* FIRMWARE_VERSION = "V1.5.1 - Integrated";

#endif
