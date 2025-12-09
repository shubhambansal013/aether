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
     * This function is non-blocking and handles the smooth color transition.
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

    // --- Constants for PWM and Color Extraction ---
    static const int PWM_MAX = 1023; // Max value for analogWrite (10-bit)
    static const int RGB_MAX = 255;  // Max value for 8-bit color
    
    // Helper to extract 8-bit component from hex
    int getR255(long hex) { return (int)((hex >> 16) & 0xFF); }
    int getG255(long hex) { return (int)((hex >> 8) & 0xFF); }
    int getB255(long hex) { return (int)(hex & 0xFF); }
    
    // --- Core Color Management ---
    void setImmediateHexColor(long hexColor);
    void startColorTransition(long newTargetColor);
    void processColorTransition();
    
    // Helper to set color on pins
    int rgbToPwm(int color255);
    void setRGBColor(int r, int g, int b); 

    // --- AQI Logic ---
    long getAqiHexColor(float pm2_5_val, bool sensorDataAvailable);
    void setAqiColor(float pm2_5_val, bool sensorDataAvailable);


    // AQI Color Hex Codes (based on EPA and prototype)
    static const long COLOR_GOOD 			    = 0x00FF00; // Bright Green (Green Full)
    static const long COLOR_MODERATE            = 0xFFFF00; // Yellow (Red + Green Full)
    static const long COLOR_UNHEALTHY_SENSITIVE = 0xFF8000; // Orange (Red Full, Green Half)
    static const long COLOR_UNHEALTHY           = 0xFF00FF; // **Pink** (Red Full, Blue Full)
    static const long COLOR_VERY_UNHEALTHY      = 0x8000FF; // Purple/Violet (Red Half, Blue Half)
    static const long COLOR_HAZARDOUS           = 0xFF0000; // **Pure Red** (Red Full)
    static const long COLOR_SENSOR_ERROR        = 0x0000FF;  // Blue (Sensor read failure)
};

#endif // RGBLED_HANDLER_H
