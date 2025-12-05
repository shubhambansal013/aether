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

class OLEDDisplay {
public:
    OLEDDisplay();
    void setup();
    void clear();
    void printMessage(String line1, String line2);
    void printMessage(String line1, String line2, String line3);
    void displaySensorDataAndWifiStatus(String wifiStatus, float pm1_0, float pm2_5, float pm10_0, float humidity, float temperature);

private:
    Adafruit_SSD1306 display;
};

#endif