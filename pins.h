#ifndef PINS_H
#define PINS_H

// --- Pin Definitions ---

// I2C OLED Display Pins (Clustered with power pins 3.3V/GND for easy PCB wiring)
// These pins (D1/D2) are close to each other on the module pinout.
const int OLED_SDA_PIN = 4; // GPIO 4 (D2)
const int OLED_SCL_PIN = 5; // GPIO 5 (D1)

// DHT22 Sensor Pin (D4 is often next to D1/D2, continuing the cluster)
const int DHT_PIN = 2;      // GPIO 2 (D4)

// UART PM Sensor Pins (Using Hardware UART for code simplicity)
// NOTE: Disconnect the sensor before uploading new code!
const int SENSOR_RX_PIN = 3;  // GPIO 3 (RX0)
const int SENSOR_TX_PIN = 1;  // GPIO 1 (TX0)

// RGB LED Pins (Clustered together on the opposite side of the module)
const int RGB_LED_RED_PIN = 12;   // GPIO 12 (D6)
const int RGB_LED_GREEN_PIN = 14; // GPIO 14 (D5)
const int RGB_LED_BLUE_PIN = 13;  // GPIO 13 (D7)

#endif
