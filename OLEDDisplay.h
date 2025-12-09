#ifndef OLEDDISPLAY_H
#define OLEDDISPLAY_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED dimensions
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

// ⭐ NEW CONSTANT: Default Brightness Level (Contrast 0-255)
const uint8_t DEFAULT_OLED_BRIGHTNESS = 1; 

class OLEDDisplay {
public:
    OLEDDisplay();
    void setup();
    void clear();
    void printMessage(String line1, String line2);
    void printMessage(String line1, String line2, String line3);
    
    /**
     * @brief Displays sensor data and status.
     * @param sensorModeChar Single character representing the current PM sensor mode (A/P/R/S).
     * @param wifiStatus Current status of the Wi-Fi connection.
     * @param pm1_0, pm2_5, pm10_0 Particulate matter readings.
     * @param humidity, temperature DHT sensor readings.
     */
    void displaySensorDataAndWifiStatus(String sensorModeChar, String wifiStatus, float pm1_0, float pm2_5, float pm10_0, float humidity, float temperature);

    /**
     * @brief Sets the display brightness (contrast).
     * @param contrast_value Contrast level (0 = dimmest, 255 = brightest).
     */
    void setBrightness(uint8_t contrast_value);

private:
    Adafruit_SSD1306 display;
    
    void drawWifiIcon(int16_t x, int16_t y, String status);

    /**
     * @brief Sends the raw I2C command to set SSD1306 contrast.
     * @param value Contrast level (0-255).
     */
    void setOLEDContrast(uint8_t value);
};

#endif
