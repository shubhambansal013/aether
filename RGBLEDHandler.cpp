#include "RGBLEDHandler.h"
#include <Arduino.h>

RGBLEDHandler::RGBLEDHandler(int rPin, int gPin, int bPin)
    : _rPin(rPin), _gPin(gPin), _bPin(bPin) {}

// --- Private Helper Method: Convert 8-bit RGB (0-255) to 10-bit PWM (0-1023) ---
int RGBLEDHandler::rgbToPwm(int color255) {
    return (int)round((float)color255 * ((float)PWM_MAX / (float)RGB_MAX));
}

// --- Private Helper Method: Apply 24-bit Hex color (0xRRGGBB) ---
void RGBLEDHandler::hexToPwm(long hexColor) {
    // 1. Extract 0-255 RGB components
    int r255 = (int)((hexColor >> 16) & 0xFF);
    int g255 = (int)((hexColor >> 8) & 0xFF);
    int b255 = (int)(hexColor & 0xFF);

    // 2. Write the values to the pins (Common Cathode: LOW = ON, HIGH = OFF)
    analogWrite(_rPin, PWM_MAX - rgbToPwm(r255));
    analogWrite(_gPin, PWM_MAX - rgbToPwm(g255));
    analogWrite(_bPin, PWM_MAX - rgbToPwm(b255));
}

// --- Private Helper Method: Set color using 0-255 components ---
void RGBLEDHandler::setRGBColor(int r, int g, int b) {
    // Common Cathode: LOW = ON, HIGH = OFF (Using PWM_MAX - PWM)
    analogWrite(_rPin, PWM_MAX - rgbToPwm(r));
    analogWrite(_gPin, PWM_MAX - rgbToPwm(g));
    analogWrite(_bPin, PWM_MAX - rgbToPwm(b));
}

// --- Private Helper Method: AQI Color Mapping ---
void RGBLEDHandler::setAqiColor(float pm2_5_val, bool sensorDataAvailable) {
    if (!sensorDataAvailable) {
        hexToPwm(COLOR_SENSOR_ERROR); // Magenta for sensor error
    } 
    // US EPA PM2.5 24-hour breakpoints (ug/m3)
    else if (pm2_5_val >= 250.5) {
        hexToPwm(COLOR_HAZARDOUS);          
    } else if (pm2_5_val >= 150.5) {
        hexToPwm(COLOR_VERY_UNHEALTHY);     
    } else if (pm2_5_val >= 55.5) {
        hexToPwm(COLOR_UNHEALTHY);          
    } else if (pm2_5_val >= 35.5) {
        hexToPwm(COLOR_UNHEALTHY_SENSITIVE);
    } else if (pm2_5_val >= 12.1) {
        hexToPwm(COLOR_MODERATE);           
    } else { // pm2_5_val <= 12.0
        hexToPwm(COLOR_GOOD);               
    }
}

// --- Public Methods ---

void RGBLEDHandler::setup() {
    pinMode(_rPin, OUTPUT);
    pinMode(_gPin, OUTPUT);
    pinMode(_bPin, OUTPUT);
    setRGBColor(0, 0, 0); // Ensure all off initially
    Serial.println("RGB LED Handler initialized.");
}

void RGBLEDHandler::startupSequence() {
    Serial.println("Running RGB Startup Sequence...");
    
    // Cycle through all colors for visual check
    long colors[] = {
        COLOR_GOOD, COLOR_MODERATE, COLOR_UNHEALTHY_SENSITIVE, 
        COLOR_UNHEALTHY, COLOR_VERY_UNHEALTHY, COLOR_HAZARDOUS,
        COLOR_SENSOR_ERROR
    };
    
    for (size_t i = 0; i < sizeof(colors) / sizeof(colors[0]); ++i) {
        hexToPwm(colors[i]);
        delay(300); // Short delay for sequence
    }
    
    hexToPwm(0x000000); // Turn off
    delay(500);
}

void RGBLEDHandler::updateLED(float pm2_5_val, bool sensorDataAvailable) {
    // The LED only cares about AQI and sensor status.
    setAqiColor(pm2_5_val, sensorDataAvailable);
}
