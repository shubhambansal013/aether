// pins.h
#ifndef PINS_H
#define PINS_H

// --- Pin Definitions ---

// RGB LED Pins (Common Cathode)
const int RGB_LED_RED_PIN = 16;   // GPIO16 (D0)
const int RGB_LED_GREEN_PIN = 0; // GPIO0 (D3)
const int RGB_LED_BLUE_PIN = 13;  // GPIO13 (D7)

// PM Sensor Pins
const int SENSOR_RX_PIN = 12; // GPIO 14 (D6)
const int SENSOR_TX_PIN = 14; // GPIO 12 (D5)

// OLED Display Pins (I2C)
const int OLED_SDA_PIN = 4; // GPIO 4 (D2)
const int OLED_SCL_PIN = 5; // GPIO 5 (D1)

// DHT22 Sensor Pin
const int DHT_PIN = 2;      // GPIO 2 (D4)

#endif
