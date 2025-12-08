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
    // *** BUG FIX: REMOVING PWM INVERSION ***
    // We assume the hardware is wired as Common Anode (High = ON, Low = OFF) 
    // OR that the PWM inversion is handled elsewhere if it's Common Cathode.
    // Based on the user observation of "whitish" color, the previous inversion logic
    // was likely causing the opposite of the desired color.
    
    analogWrite(_rPin, rgbToPwm(r)); 
    analogWrite(_gPin, rgbToPwm(g)); 
    analogWrite(_bPin, rgbToPwm(b)); 
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
        setImmediateHexColor(_targetColorHex);
        return;
    }

    // --- Linear Interpolation (LERP) ---

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
    Serial.println("Running RGB Startup Sequence (R -> G -> B fade)...");
    
    // Define the required sequence: Red, Green, Blue
    const long STARTUP_COLORS[] = {
        0xFF0000L, // Red
        0x00FF00L, // Green
        0x0000FFL  // Blue
    };
    const size_t NUM_COLORS = sizeof(STARTUP_COLORS) / sizeof(STARTUP_COLORS[0]);

    // 1. Start the sequence with an initial black color to ensure fade-in
    long currentSequenceColor = 0x000000L;
    
    // 2. Iterate through all target colors, fading to each one
    for (size_t i = 0; i < NUM_COLORS; ++i) {
        // Start the transition from the previous color to the current target color
        currentSequenceColor = STARTUP_COLORS[i];
        startColorTransition(currentSequenceColor);
        
        unsigned long transitionStart = millis();
        
        // Blocking wait for the full transition time
        while (millis() - transitionStart < FADE_DURATION_MS) {
            processColorTransition();
            delay(1); // Small delay to yield time and prevent watchdog
        }
        
        // Ensure final color is set and wait for the display period
        setImmediateHexColor(currentSequenceColor); 
        delay(1000); // Pause on color for 1 second
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
