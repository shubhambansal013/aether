#ifndef RGBLED_HANDLER_H
#define RGBLED_HANDLER_H

#include <Arduino.h>

class RGBLEDHandler {
public:
    // Define DiagnosticStatus enum here or ensure it's accessible
    enum DiagnosticStatus {
        STATUS_PING_SUCCESS = 1,
        STATUS_PING_FAILURE = 2
    };

    RGBLEDHandler(int rPin, int gPin, int bPin);
    void setup();
    void setRGBColor(int r, int g, int b);
    void startBlink(DiagnosticStatus status);
    void updateLED(bool isWifiConnected, float pm2_5_val, bool sensorDataAvailable);

private:
    int _rPin;
    int _gPin;
    int _bPin;
    unsigned long _ledStateTime = 0;
    bool _ledState = HIGH;
    int _blinkCount = 0;
    bool _isBlinking = false;
    
    // Last known PM2.5 value for color logic
    float _lastPm2_5_val = 0.0; 
    bool _lastSensorDataAvailable = false;
    bool _isPulsing = false;
    unsigned long _pulseStartTime = 0;
};

#endif // RGBLED_HANDLER_H
