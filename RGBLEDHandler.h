#ifndef RGBLED_HANDLER_H
#define RGBLED_HANDLER_H

#include <Adafruit_NeoPixel.h>
#include "SystemData.h"

class RGBLEDHandler {
public:
    RGBLEDHandler(int pin);
    void setup();
    void startupSequence();
    void updateLED(float pm2_5); 
    void turnOff(); // Added for Stealth Mode

private:
    Adafruit_NeoPixel _strip;
    void setLEDColor(uint32_t hex);
    uint32_t getColorForConcentration(float pm2_5);
    
    static const uint32_t C_GREEN  = 0x00FF00;
    static const uint32_t C_YELLOW = 0xFFFF00;
    static const uint32_t C_ORANGE = 0xFF8000;
    static const uint32_t C_PINK   = 0xFF1493;
    static const uint32_t C_PURPLE = 0x800080;
    static const uint32_t C_RED    = 0xFF0000;
    static const uint32_t C_BLUE   = 0x0000FF;
};

#endif
