#ifndef RGBLED_HANDLER_H
#define RGBLED_HANDLER_H

#include <Arduino.h>

class RGBLEDHandler {
public:
    RGBLEDHandler(int rPin, int gPin, int bPin);
    void setup();
    
    /**
     * @brief Runs a sequence of all AQI colors for initialization/debug with fades.
     */
    void startupSequence();

    /**
     * @brief Updates the LED color based only on PM2.5 level and sensor availability.
     * @param pm2_5_val The measured PM2.5 concentration (ug/m3).
     * @param sensorDataAvailable True if PM sensor data is valid.
     */
    void updateLED(float pm2_5_val, bool sensorDataAvailable);

private:
    int _rPin;
    int _gPin;
    int _bPin;

    // --- Transition Variables ---
    long _currentColorHex = 0x000000;
    long _targetColorHex = 0x000000;
    unsigned long _transitionStartTime = 0;
    const unsigned long FADE_DURATION_MS = 1000; // 1 second transition time

    // --- Constants for PWM and AQI Colors ---
    static const int PWM_MAX = 1023; // Max value for analogWrite
    static const int RGB_MAX = 255;  // Max value for 8-bit color
    
    // Helper to extract 8-bit component from hex
    int getR255(long hex) { return (int)((hex >> 16) & 0xFF); }
    int getG255(long hex) { return (int)((hex >> 8) & 0xFF); }
    int getB255(long hex) { return (int)(hex & 0xFF); }
    
    // Core color setting methods
    void setImmediateHexColor(long hexColor);
    void startColorTransition(long newTargetColor);
    void processColorTransition();
    
    // Helper to convert 8-bit RGB (0-255) to 10-bit PWM (0-1023)
    int rgbToPwm(int color255);
    void setRGBColor(int r, int g, int b); // Helper to set color from 0-255

    // AQI Logic
    void setAqiColor(float pm2_5_val, bool sensorDataAvailable);
    long getAqiHexColor(float pm2_5_val, bool sensorDataAvailable);


    // AQI Color Hex Codes (unchanged)
    static const long COLOR_GOOD                = 0x00FF00;
    static const long COLOR_MODERATE            = 0xFFFF00;
    static const long COLOR_UNHEALTHY_SENSITIVE = 0xFF8000;
    static const long COLOR_UNHEALTHY           = 0xFF0000;
    static const long COLOR_VERY_UNHEALTHY      = 0x800080;
    static const long COLOR_HAZARDOUS           = 0x80000A;
    static const long COLOR_SENSOR_ERROR        = 0xFF00FF;
};

#endif // RGBLED_HANDLER_H
