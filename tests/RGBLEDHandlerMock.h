#ifndef RGBLEDHANDLER_MOCK_H
#define RGBLEDHANDLER_MOCK_H

#include "Arduino.h"
#include "Adafruit_NeoPixel.h"

class RGBLEDHandler {
public:
    RGBLEDHandler(int pin);
    void setup();
    void updateLED(float pm);
    void turnOff();
    void setColor(uint32_t hex);
    void startupSequence();

    int updateCount = 0;
    int turnOffCount = 0;
    float lastPM = 0;
private:
    Adafruit_NeoPixel _strip;
};

#endif
