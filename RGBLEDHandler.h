#ifndef RGBLED_HANDLER_H
#define RGBLED_HANDLER_H

#include <Arduino.h>

class RGBLEDHandler {
public:
    RGBLEDHandler(int rPin, int gPin, int bPin);
    void setup();
    
    /**
     * @brief Runs a sequence of all AQI colors for initialization/debug.
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

    // --- Constants for PWM and AQI Colors ---
    static const int PWM_MAX = 1023; // Max value for analogWrite
    static const int RGB_MAX = 255;  // Max value for 8-bit color
    
    // Helper to convert 8-bit RGB (0-255) to 10-bit PWM (0-1023)
    int rgbToPwm(int color255);
    
    // Helper to apply 24-bit Hex color (0xRRGGBB)
    void hexToPwm(long hexColor);

    // Color Setting Logic
    void setAqiColor(float pm2_5_val, bool sensorDataAvailable);
    void setRGBColor(int r, int g, int b); // Helper to set color from 0-255

    // AQI Color Hex Codes (based on EPA and prototype)
    static const long COLOR_GOOD                = 0x00FF00;  // Green (0.0-12.0)
    static const long COLOR_MODERATE            = 0xFFFF00;  // Yellow (12.1-35.4)
    static const long COLOR_UNHEALTHY_SENSITIVE = 0xFF8000;  // Orange (35.5-55.4)
    static const long COLOR_UNHEALTHY           = 0xFF0000;  // Red (55.5-150.4)
    static const long COLOR_VERY_UNHEALTHY      = 0x800080;  // Purple (150.5-250.4)
    static const long COLOR_HAZARDOUS           = 0x80000A;  // Maroon (250.5+)
    static const long COLOR_SENSOR_ERROR        = 0xFF00FF;  // Magenta
    // COLOR_WIFI_DISCONNECTED is no longer needed
};

#endif // RGBLED_HANDLER_H
