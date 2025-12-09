// pins.h
#ifndef PINS_H
#define PINS_H

// --- Pin Definitions ---

// PM Sensor Pin (Single SoftwareSerial RX line on D5)
// This uses a non-conflicting pin, avoiding the D3/GPIO3 upload issue.
const int PM_SENSOR_RX_PIN = 14; // GPIO 14 (D5) -> Connected to Sensor TX (Data Out)
const int PM_SENSOR_TX_PIN = 0; // GPIO 0 (D3) -> Connected to Sensor RX (Data in)

// RGB LED Pins (Common Cathode)
// Clustered on D6, D7, and D8 for clean wiring.
// NOTE: D8 (GPIO15) REQUIRES a 10k ohm PULL-DOWN RESISTOR to GND for boot stability!
// NOTE: All 3 pins require individual 220 ohm CURRENT-LIMITING RESISTORS.
const int RGB_LED_RED_PIN   = 13; // GPIO 12 (D7) 
const int RGB_LED_GREEN_PIN = 15; // GPIO 13 (D8)
const int RGB_LED_BLUE_PIN  = 12; // GPIO 15 (D6) 

// OLED Display Pins (I2C)
// Clustered on standard, safe I2C pins D1 and D2.
const int OLED_SDA_PIN = 4; // GPIO 4 (D2)
const int OLED_SCL_PIN = 5; // GPIO 5 (D1)

// DHT22 Sensor Pin
// On the safe, flexible D4 pin.
const int DHT_PIN = 2;      // GPIO 2 (D4)

#endif
