#include "RGBLEDHandler.h"
#include <Arduino.h>

RGBLEDHandler::RGBLEDHandler(int rPin, int gPin, int bPin)
    : _rPin(rPin), _gPin(gPin), _bPin(bPin) {}

void RGBLEDHandler::setup() {
    pinMode(_rPin, OUTPUT);
    pinMode(_gPin, OUTPUT);
    pinMode(_bPin, OUTPUT);
    // Ensure all off initially
    setRGBColor(0, 0, 0); 
    Serial.println("RGB LED Handler initialized.");
}

void RGBLEDHandler::setRGBColor(int r, int g, int b) {
    analogWrite(_rPin, 255 - r); // Common Cathode: LOW = ON, HIGH = OFF
    analogWrite(_gPin, 255 - g);
    analogWrite(_bPin, 255 - b);
}

void RGBLEDHandler::startBlink(DiagnosticStatus status) {
    _isPulsing = false; // Stop pulsing if blinking starts
    _blinkCount = (int)status * 2;
    _isBlinking = true;
    _ledStateTime = millis();
    setRGBColor(0, 0, 0); // Start with LED ON (effectively LOW for common cathode, but set to black for control)
    _ledState = LOW; // Represents internal state for blinking logic, not actual color
}

void RGBLEDHandler::updateLED(bool isWifiConnected, float pm2_5_val, bool sensorDataAvailable) {
    // Handle blinking for ping status first
    if (_isBlinking) {
        if (millis() - _ledStateTime >= 150) {
            _ledState = !_ledState;
            if (_ledState == LOW) { // LED ON
                if (_blinkCount % 2 != 0) { // Red for failure, Green for success
                     // Based on startBlink status, this assumes odd blinkCount means failure and even means success after decrementing
                    if (_blinkCount == (int)STATUS_PING_FAILURE * 2 - 1 || _blinkCount == (int)STATUS_PING_FAILURE * 2 - 3) { // Simplified for two blinks
                        setRGBColor(255, 0, 0); // Red for failure
                    } else {
                        setRGBColor(0, 255, 0); // Green for success
                    }    
                } else { // LED OFF
                    setRGBColor(0, 0, 0); // Off
                }
            } else { // LED OFF
                 setRGBColor(0, 0, 0); // Off
            }
            _ledStateTime = millis();
            _blinkCount--;

            if (_blinkCount <= 0) {
                _isBlinking = false;
                // After blinking, revert to regular status color
                _ledStateTime = 0; // Reset for next state
            }
        }
        return; // Exit if currently blinking
    }

    // Handle WiFi status (highest priority after blinking)
    if (!isWifiConnected) {
        // Pulsing Blue for WiFi connecting or AP mode
        if (!_isPulsing) {
            _isPulsing = true;
            _pulseStartTime = millis();
        }
        unsigned long pulseTime = millis() - _pulseStartTime;
        int brightness = map(sin(pulseTime / 500.0), -1, 1, 30, 200); // Slow sine wave pulse
        setRGBColor(0, 0, brightness); // Blue pulse
        return;
    } else {
        _isPulsing = false; // Stop pulsing once connected
    }

    // Handle Air Quality (only if WiFi is connected and not blinking)
    if (!sensorDataAvailable) {
        setRGBColor(255, 0, 255); // Magenta for sensor error
    } else {
        if (pm2_5_val < 12.0) {
            setRGBColor(0, 255, 0); // Green: Good
        } else if (pm2_5_val >= 12.0 && pm2_5_val < 35.0) {
            setRGBColor(255, 255, 0); // Yellow: Moderate (Red + Green)
        } else {
            setRGBColor(255, 0, 0); // Red: Bad
        }
    }
}
