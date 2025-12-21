#include "RGBLEDHandler.h"

// Shield for compiler bugs
#ifndef IRAM_ATTR
  #define IRAM_ATTR __attribute__((section(".text")))
#endif

RGBLEDHandler::RGBLEDHandler(int pin) : _strip(1, pin, NEO_GRB + NEO_KHZ800) {}

void RGBLEDHandler::setup() {
    _strip.begin();
    _strip.setBrightness(50);
    _strip.show();
}

void RGBLEDHandler::setLEDColor(uint32_t hex) {
    _strip.setPixelColor(0, hex);
    _strip.show();
}

void RGBLEDHandler::startupSequence() {
    setLEDColor(C_RED);   delay(500);
    setLEDColor(C_GREEN); delay(500);
    setLEDColor(C_BLUE);  delay(500);
    setLEDColor(0);
}

void RGBLEDHandler::updateLED(float pm2_5) {
    // ONLY show Blue if we have never received a single packet (pm2_5 is -1)
    if (pm2_5 < 0) { 
        setLEDColor(C_BLUE); 
        return; 
    }
    
    uint32_t color;
    if (pm2_5 <= 30)       color = C_GREEN;
    else if (pm2_5 <= 60)  color = C_YELLOW;
    else if (pm2_5 <= 90)  color = C_ORANGE;
    else if (pm2_5 <= 120) color = C_PINK;
    else if (pm2_5 <= 250) color = C_PURPLE;
    else                   color = C_RED;
    
    setLEDColor(color);
}
