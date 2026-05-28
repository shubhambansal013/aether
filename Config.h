#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- Pins (NodeMCU ESP8266) ---
static const int PM_SENSOR_RX_PIN  = 14; // D5 (GPIO 14)
static const int PM_SENSOR_SET_PIN = 12; // D6 (GPIO 12)
static const int OLED_SDA_PIN      = 4;  // D2 (GPIO 4)
static const int OLED_SCL_PIN      = 5;  // D1 (GPIO 5)
static const int DHT_PIN           = 2;  // D4 (GPIO 2)
static const int WS2812_PIN        = 15; // D8 (GPIO 15)
static const int BUTTON_PIN        = 13; // D7 (GPIO 13)

// --- Default Settings ---
static const int DEFAULT_MODE_SETTING = 1;
static const uint8_t DEFAULT_OLED_BRIGHTNESS = 150;

// --- EEPROM Addresses ---
static const int ADDR_MODE  = 10;
static const int ADDR_MUTED = 14;

// --- Timing Constants (in Milliseconds) ---
static const unsigned long INITIAL_WARMUP_DURATION = 300000; 
static const unsigned long PM_WAKE_DURATION        = 32000;  
static const unsigned long PM_SLEEP_DURATION       = 60000; 
static const unsigned long STABILITY_THRESHOLD      = 30000;  
static const unsigned long BLYNK_SEND_INTERVAL      = 60000;  
static const unsigned long BLYNK_ICON_KEEP_ALIVE    = 3000;   

// Button Timing
static const unsigned long DEBOUNCE_DELAY          = 50;
static const unsigned long BUTTON_LONG_PRESS_TIME  = 2000; 

// --- OTA Settings ---
static const unsigned long OTA_CHECK_INTERVAL = 60000; // 1 min for testing — change to 3600000 (1hr) before production
static const char* OTA_MANIFEST_URL = "https://shubhambansal013.github.io/aether/version.json";

// --- Meta ---
static const char* FIRMWARE_VERSION = "dev"; // overwritten by CI via sed before compile — do not change manually

#endif
