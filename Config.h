#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ======================================================================
// 📌 PIN DEFINITIONS (Optimized for NodeMCU)
// ======================================================================

// PM Sensor (D5, D6)
const int PM_SENSOR_RX_PIN  = 14; // GPIO 14 (D5) -> Sensor TX
const int PM_SENSOR_SET_PIN = 0; // GPIO 0 (D3) -> Sensor SET

// RGB LED (PWM - Common Cathode)
const int RGB_LED_RED_PIN   = 13; // GPIO 13 (D7)
const int RGB_LED_GREEN_PIN = 15; // GPIO 15 (D8) - Needs 10k Pull-down
const int RGB_LED_BLUE_PIN  = 12;  // GPIO 12  (D6) - Needs 10k Pull-up

// OLED I2C (D1, D2)
const int OLED_SDA_PIN = 4; // GPIO 4 (D2)
const int OLED_SCL_PIN = 5; // GPIO 5 (D1)

// DHT22 (D4)
const int DHT_PIN = 2; // GPIO 2 (D4)

// ======================================================================
// ⏱️ TIMING & STABILITY CONFIGURATION
// ======================================================================

// The 5-minute initial run to establish a baseline
const unsigned long INITIAL_WARMUP_DURATION = 300000; 

// Duty Cycle: How long to stay awake vs sleep
const unsigned long PM_WAKE_DURATION  = 60000;  // 60s total run time
const unsigned long PM_SLEEP_DURATION = 60000; // 1 minute sleep

// Reliability: Only "trust" data after the fan has purged the chamber
// Since you saw values climbing until the end, we wait for 10 seconds.
const unsigned long STABILITY_THRESHOLD = 10000; 

// Cloud & UI
const unsigned long BLYNK_SEND_INTERVAL = 60000;
const unsigned long BLYNK_ICON_KEEP_ALIVE = 3000;

// ======================================================================
// 🌐 FIRMWARE & CLOUD INFO
// ======================================================================
const char* FIRMWARE_VERSION = "V1.3.3 - Config Refactor";

#endif
