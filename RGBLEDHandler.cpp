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

void RGBLEDHandler::turnOff() {
    setLEDColor(0);
}

void RGBLEDHandler::setColor(uint32_t hex) {
    setLEDColor(hex);
}

void RGBLEDHandler::startupSequence() {
    setLEDColor(0xFF0000); delay(500); // Red
    setLEDColor(0x00FF00); delay(500); // Green
    setLEDColor(0x0000FF); delay(500); // Blue
    turnOff();
}

void RGBLEDHandler::updateLED(float pm2_5) {
    uint32_t color = getColorForConcentration(pm2_5);
    setLEDColor(color);
}

uint32_t RGBLEDHandler::getColorForConcentration(float pm2_5) {
    if (pm2_5 < 0) return C_BLUE;
    
    if (pm2_5 <= 30)       return C_GREEN;
    if (pm2_5 <= 60)       return C_YELLOW;
    if (pm2_5 <= 90)       return C_ORANGE;
    if (pm2_5 <= 120)      return C_PINK;
    if (pm2_5 <= 250)      return C_PURPLE;
    
    return C_RED;
}
