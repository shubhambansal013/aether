#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- Pins (NodeMCU ESP8266) ---
const int PM_SENSOR_RX_PIN  = 14; // D5 (GPIO 14)
const int PM_SENSOR_SET_PIN = 12; // D6 (GPIO 12)
const int OLED_SDA_PIN      = 4;  // D2 (GPIO 4)
const int OLED_SCL_PIN      = 5;  // D1 (GPIO 5)
const int DHT_PIN           = 2;  // D4 (GPIO 2)
const int WS2812_PIN        = 15; // D8 (GPIO 15)
const int BUTTON_PIN        = 13; // D7 (GPIO 13)

// --- Default Settings ---
const int DEFAULT_MODE_SETTING = 1;
const uint8_t DEFAULT_OLED_BRIGHTNESS = 150;

// --- Timing Constants (in Milliseconds) ---
const unsigned long INITIAL_WARMUP_DURATION = 300000; 
const unsigned long PM_WAKE_DURATION        = 32000;  
const unsigned long PM_SLEEP_DURATION       = 60000; 
const unsigned long STABILITY_THRESHOLD      = 30000;  
const unsigned long BLYNK_SEND_INTERVAL      = 60000;  
const unsigned long BLYNK_ICON_KEEP_ALIVE    = 3000;   

// Button Timing
const unsigned long DEBOUNCE_DELAY          = 50;
const unsigned long BUTTON_LONG_PRESS_TIME  = 2000; // 2 seconds to toggle OLED/LED

// --- Meta ---
const char* FIRMWARE_VERSION = "V1.5.1 - Integrated";

#endif
