#include "RGBLEDHandler.h"
#include <Arduino.h>

RGBLEDHandler::RGBLEDHandler(int rPin, int gPin, int bPin)
    : _rPin(rPin), _gPin(gPin), _bPin(bPin) {}

// --- Private Helper Method: PWM and Color Setting ---

int RGBLEDHandler::rgbToPwm(int color255) {
    // Convert 8-bit (0-255) to 10-bit (0-1023)
    return (int)round((float)color255 * ((float)PWM_MAX / (float)RGB_MAX));
}

void RGBLEDHandler::setRGBColor(int r, int g, int b) {
    // Set color on pins (Common Cathode: LOW = ON, HIGH = OFF)
    // We use PWM_MAX - PWM to invert the signal for common cathode.
    analogWrite(_rPin, PWM_MAX - rgbToPwm(r));
    analogWrite(_gPin, PWM_MAX - rgbToPwm(g));
    analogWrite(_bPin, PWM_MAX - rgbToPwm(b));
}

/**
 * @brief Sets color immediately without transition, updating the current color state.
 */
void RGBLEDHandler::setImmediateHexColor(long hexColor) {
    _currentColorHex = hexColor;
    _targetColorHex = hexColor; // Reset target
    _transitionStartTime = 0;   // Reset transition state
    setRGBColor(getR255(hexColor), getG255(hexColor), getB255(hexColor));
}

/**
 * @brief Starts a new color transition to the specified target.
 */
void RGBLEDHandler::startColorTransition(long newTargetColor) {
    if (_targetColorHex != newTargetColor) {
        // Only start a new transition if the target is different from the current target
        // The current color state already holds the right start color (it might be mid-transition)
        _currentColorHex = (_transitionStartTime == 0) ? _targetColorHex : _currentColorHex;
        _targetColorHex = newTargetColor;
        _transitionStartTime = millis();
    }
}

/**
 * @brief Performs non-blocking linear interpolation between the start and target colors.
 */
void RGBLEDHandler::processColorTransition() {
    if (_transitionStartTime == 0) return; // No transition active

    unsigned long elapsedTime = millis() - _transitionStartTime;
    float progress = (float)elapsedTime / FADE_DURATION_MS;

    if (progress >= 1.0) {
        // Transition complete: Set final color and reset state
        setImmediateHexColor(_targetColorHex);
        return;
    }

    // --- Linear Interpolation (LERP) ---
    // Color = Start + (Target - Start) * Progress

    // Red component
    int rStart = getR255(_currentColorHex);
    int rTarget = getR255(_targetColorHex);
    int rCurrent = rStart + (int)((float)(rTarget - rStart) * progress);

    // Green component
    int gStart = getG255(_currentColorHex);
    int gTarget = getG255(_targetColorHex);
    int gCurrent = gStart + (int)((float)(gTarget - gStart) * progress);

    // Blue component
    int bStart = getB255(_currentColorHex);
    int bTarget = getB255(_targetColorHex);
    int bCurrent = bStart + (int)((float)(bTarget - bStart) * progress);

    // Apply the interpolated color
    setRGBColor(rCurrent, gCurrent, bCurrent);
}

// --- Private Helper Method: AQI Color Mapping ---

/**
 * @brief Returns the appropriate Hex color code based on PM2.5 level.
 */
long RGBLEDHandler::getAqiHexColor(float pm2_5_val, bool sensorDataAvailable) {
    if (!sensorDataAvailable) {
        return COLOR_SENSOR_ERROR;
    } 
    // US EPA PM2.5 24-hour breakpoints (ug/m3)
    else if (pm2_5_val >= 250.5) {
        return COLOR_HAZARDOUS;          
    } else if (pm2_5_val >= 150.5) {
        return COLOR_VERY_UNHEALTHY;     
    } else if (pm2_5_val >= 55.5) {
        return COLOR_UNHEALTHY;          
    } else if (pm2_5_val >= 35.5) {
        return COLOR_UNHEALTHY_SENSITIVE;
    } else if (pm2_5_val >= 12.1) {
        return COLOR_MODERATE;           
    } else { // pm2_5_val <= 12.0
        return COLOR_GOOD;               
    }
}

void RGBLEDHandler::setAqiColor(float pm2_5_val, bool sensorDataAvailable) {
    long newColor = getAqiHexColor(pm2_5_val, sensorDataAvailable);
    startColorTransition(newColor);
}

// --- Public Methods ---

void RGBLEDHandler::setup() {
    pinMode(_rPin, OUTPUT);
    pinMode(_gPin, OUTPUT);
    pinMode(_bPin, OUTPUT);
    setImmediateHexColor(0x000000); // Ensure all off initially and set state
    Serial.println("RGB LED Handler initialized.");
}

void RGBLEDHandler::startupSequence() {
    Serial.println("Running RGB Startup Sequence...");
    
    long colors[] = {
        COLOR_GOOD, COLOR_MODERATE, COLOR_UNHEALTHY_SENSITIVE, 
        COLOR_UNHEALTHY, COLOR_VERY_UNHEALTHY, COLOR_HAZARDOUS,
        COLOR_SENSOR_ERROR
    };

    // 1. Fade in the first color
    startColorTransition(colors[0]);
    unsigned long sequenceStartTime = millis();
    while (millis() - sequenceStartTime < FADE_DURATION_MS) {
        processColorTransition();
        // Use a short delay in blocking functions to prevent watchdog timer trips on some platforms
        delay(1); 
    }
    setImmediateHexColor(colors[0]);

    // 2. Fade through the rest of the colors
    for (size_t i = 1; i < sizeof(colors) / sizeof(colors[0]); ++i) {
        startColorTransition(colors[i]);
        unsigned long transitionStart = millis();
        // Blocking wait for transition to complete
        while (millis() - transitionStart < FADE_DURATION_MS) {
            processColorTransition();
            delay(1); 
        }
        setImmediateHexColor(colors[i]);
        delay(500); // Pause on color
    }
    
    // 3. Fade out to black
    startColorTransition(0x000000);
    unsigned long fadeOutStart = millis();
    while (millis() - fadeOutStart < FADE_DURATION_MS) {
        processColorTransition();
        delay(1); 
    }
    setImmediateHexColor(0x000000); 
    delay(500);
}

void RGBLEDHandler::updateLED(float pm2_5_val, bool sensorDataAvailable) {
    // 1. Check for the required AQI color and start transition if needed
    setAqiColor(pm2_5_val, sensorDataAvailable);
    
    // 2. Process the transition (non-blocking)
    processColorTransition();
}
