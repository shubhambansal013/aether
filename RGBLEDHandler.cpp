#include "RGBLEDHandler.h"

#ifndef IRAM_ATTR
  #define IRAM_ATTR __attribute__((section(".text")))
#endif

RGBLEDHandler::RGBLEDHandler(int pin) : _strip(1, pin, NEO_GRB + NEO_KHZ800) {}

void RGBLEDHandler::setup() {
    pinMode(15, OUTPUT); // Explicitly set D8/GPIO15
    digitalWrite(15, LOW);
    _strip.begin();
    _strip.setBrightness(50);
    _strip.show();
}

void RGBLEDHandler::setLEDColor(uint32_t hex) {
    _strip.setPixelColor(0, hex);
    _strip.show();
}

void RGBLEDHandler::startupSequence() {
    setLEDColor(0xFF0000); delay(500); // Red
    setLEDColor(0x00FF00); delay(500); // Green
    setLEDColor(0x0000FF); delay(500); // Blue
    setLEDColor(0);
}

void RGBLEDHandler::updateLED(float pm2_5, bool enabled) {
    if (!enabled) {
        setLEDColor(0);
        return;
    }

    if (pm2_5 < 0) { 
        setLEDColor(C_BLUE); // Stay blue if no sensor data received yet
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
