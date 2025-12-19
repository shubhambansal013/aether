#ifndef PINS_H
#define PINS_H

// --- PM Sensor Pins ---
// Using Hardware Control (SET pin) instead of Serial TX
const int PM_SENSOR_RX_PIN  = 14; // GPIO 14 (D5) -> Connected to Sensor TX (Data Out)
const int PM_SENSOR_SET_PIN = 0; // GPIO 12 (D3) -> Connected to Sensor SET (Wake/Sleep)

// --- RGB LED Pins (Common Cathode) ---
const int RGB_LED_RED_PIN   = 13;
const int RGB_LED_GREEN_PIN = 15; 
const int RGB_LED_BLUE_PIN  = 12;  

// --- OLED Display Pins (I2C) ---
const int OLED_SDA_PIN = 4; // GPIO 4 (D2)
const int OLED_SCL_PIN = 5; // GPIO 5 (D1)

// --- DHT22 Sensor Pin ---
const int DHT_PIN = 2;      // GPIO 2 (D4)

#endif
