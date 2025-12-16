#ifndef PINS_H
#define PINS_H

// --- Pin Definitions ---

// PM Sensor Pins
const int PM_SENSOR_RX_PIN = 14; // GPIO 14 (D5) -> Connected to Sensor TX
const int PM_SENSOR_TX_PIN = 0;  // GPIO 0 (D3)  -> Connected to Sensor RX

// WS2812 (NeoPixel) LED Pin
// Only one pin is required for WS2812. 
// We use GPIO 13 (D7) because it is stable during boot.
const int WS2812_DATA_PIN = 13; 

// OLED Display Pins (I2C)
const int OLED_SDA_PIN = 4; // GPIO 4 (D2)
const int OLED_SCL_PIN = 5; // GPIO 5 (D1)

// DHT22 Sensor Pin
const int DHT_PIN = 2;      // GPIO 2 (D4)

#endif
