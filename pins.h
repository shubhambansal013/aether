#ifndef PINS_H
#define PINS_H

// --- PM Sensor Pins ---
// Using Hardware Control (SET pin) instead of Serial TX
const int PM_SENSOR_RX_PIN  = 14; // GPIO 14 (D5) -> Connected to Sensor TX (Data Out)
const int PM_SENSOR_SET_PIN = 0; // GPIO 12 (D3) -> Connected to Sensor SET (Wake/Sleep)

// --- RGB LED Pins (Common Cathode) ---
// Note: D8 (GPIO 15) MUST have a 10k Pull-down to GND for the ESP8266 to boot.
const int RGB_LED_RED_PIN   = 12; // GPIO 13 (D6)
const int RGB_LED_GREEN_PIN = 13; // GPIO 15 (D7)
const int RGB_LED_BLUE_PIN  = 15;  // GPIO 0  (D8) - Moved here to free up D6 for Sensor SET

// --- OLED Display Pins (I2C) ---
const int OLED_SDA_PIN = 4; // GPIO 4 (D2)
const int OLED_SCL_PIN = 5; // GPIO 5 (D1)

// --- DHT22 Sensor Pin ---
const int DHT_PIN = 2;      // GPIO 2 (D4)

#endif
