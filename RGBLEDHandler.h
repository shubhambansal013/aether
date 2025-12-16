#ifndef RGBLED_HANDLER_H
#define RGBLED_HANDLER_H

#include <Arduino.h>

class RGBLEDHandler {
public:
    // Only one pin needed for WS2812 Data
    RGBLEDHandler(int dataPin);
    
    void setup();
    void startupSequence();
    void updateLED(float pm2_5_val, bool sensorDataAvailable);

private:
    int _dataPin;

    // Helper to set color directly
    void setImmediateHexColor(long hexColor);
    long getAqiHexColor(float pm2_5_val, bool sensorDataAvailable);

    // AQI Color Hex Codes
    static const long COLOR_GOOD                = 0x00FF00; // Green
    static const long COLOR_MODERATE            = 0xFFFF00; // Yellow
    static const long COLOR_UNHEALTHY_SENSITIVE = 0xFF8000; // Orange
    static const long COLOR_UNHEALTHY           = 0xFF00FF; // Pink
    static const long COLOR_VERY_UNHEALTHY      = 0x8000FF; // Purple
    static const long COLOR_HAZARDOUS           = 0xFF0000; // Red
    static const long COLOR_SENSOR_ERROR        = 0x0000FF; // Blue
};

#endif
