#ifndef OLEDDISPLAY_H
#define OLEDDISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "SystemData.h"

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
    
    // Clean interface using the Data Object
    void update(const SystemData& data);

private:
    Adafruit_SSD1306 display;
    
    void drawStatusBar(const SystemData& data);
    void drawHeroSection(float pm2_5);
    void drawSecondaryGrid(const SystemData& data);
    
    void drawWifiIcon(int16_t x, int16_t y, String status);
    void drawModeIcon(int16_t x, int16_t y, bool isWarmup);
    void drawFanIcon(int16_t x, int16_t y, bool isFanOn);
    void setOLEDContrast(uint8_t value);
};

#endif
