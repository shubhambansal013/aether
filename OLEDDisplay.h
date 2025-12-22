#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "SystemData.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define DEFAULT_OLED_BRIGHTNESS 150

class OLEDDisplay {
public:
    OLEDDisplay(int sdaPin, int sclPin);
    void setup();
    void update(const SystemData& data);
    void setBrightness(uint8_t val);
    void clear();
    void printMessage(String l1, String l2);

private:
    Adafruit_SSD1306 display;
    int _sda, _scl;

    // --- Refactored Private Drawing Methods ---
    void drawStatusBar(const SystemData& data);
    void drawHeroSection(float pm2_5);
    void drawSecondaryGrid(const SystemData& data);
    
    // NEW: These were missing from your header
    void renderMetric(int x, int y, String label, float val, int precision, String unit = "");
    void drawBlynkIndicator();

    // --- UI Components ---
    void drawWifiIcon(int16_t x, int16_t y, String status);
    void drawModeIcon(int16_t x, int16_t y, bool isWarmup);
    void drawFanIcon(int16_t x, int16_t y, bool isFanOn);
    void setOLEDContrast(uint8_t val);
};

#endif
