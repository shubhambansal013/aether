#ifndef OLEDDISPLAY_H
#define OLEDDISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define DEFAULT_OLED_BRIGHTNESS 100

class OLEDDisplay {
public:
    OLEDDisplay();
    void setup();
    void setBrightness(uint8_t contrast_value);
    void clear();
    void printMessage(String line1, String line2);

    // The single point of entry for rendering the dashboard
    void update(String wifiStatus, float pm1_0, float pm2_5, float pm10_0, 
                float temp, float hum, bool isWarmup, bool isSleeping);

private:
    Adafruit_SSD1306 display;
    
    // Internal Rendering Sections
    void drawStatusBar(String wifiStatus, bool isWarmup, bool isSleeping);
    void drawHeroSection(float pm2_5);
    void drawSecondaryGrid(float pm1_0, float pm10_0, float temp, float hum);
    
    // Icon Drawing Helpers
    void drawWifiIcon(int16_t x, int16_t y, String status);
    void drawModeIcon(int16_t x, int16_t y, bool isWarmup);
    void drawFanIcon(int16_t x, int16_t y, bool isFanOn);
    void setOLEDContrast(uint8_t value);
};

#endif
