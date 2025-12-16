#include "RGBLEDHandler.h"
#include <Adafruit_NeoPixel.h>

#define LED_COUNT 1 // Set this to the number of LEDs you have

// Initialize the NeoPixel strip object
Adafruit_NeoPixel strip(LED_COUNT, 0, NEO_GRB + NEO_KHZ800);

RGBLEDHandler::RGBLEDHandler(int dataPin) : _dataPin(dataPin) {}

void RGBLEDHandler::setup() {
    strip.setPin(_dataPin);
    strip.begin();
    strip.setBrightness(150); // Set brightness to ~60% to save power/eyes
    strip.show();             // Initialize all pixels to 'off'
    Serial.println("WS2812 LED Handler initialized.");
}

void RGBLEDHandler::setImmediateHexColor(long hexColor) {
    uint8_t r = (uint8_t)(hexColor >> 16);
    uint8_t g = (uint8_t)(hexColor >> 8);
    uint8_t b = (uint8_t)(hexColor);

    for(int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
}

long RGBLEDHandler::getAqiHexColor(float pm2_5_val, bool sensorDataAvailable) {
    if (!sensorDataAvailable) return COLOR_SENSOR_ERROR;

    if (pm2_5_val >= 250.5)     return COLOR_HAZARDOUS;
    else if (pm2_5_val >= 150.5) return COLOR_VERY_UNHEALTHY;
    else if (pm2_5_val >= 55.5)  return COLOR_UNHEALTHY;
    else if (pm2_5_val >= 35.5)  return COLOR_UNHEALTHY_SENSITIVE;
    else if (pm2_5_val >= 12.1)  return COLOR_MODERATE;
    else                         return COLOR_GOOD;
}

void RGBLEDHandler::updateLED(float pm2_5_val, bool sensorDataAvailable) {
    long color = getAqiHexColor(pm2_5_val, sensorDataAvailable);
    setImmediateHexColor(color);
}

void RGBLEDHandler::startupSequence() {
    Serial.println("Running Startup Sequence (R-G-B)...");
    long colors[] = {0xFF0000, 0x00FF00, 0x0000FF};
    
    for (int i = 0; i < 3; i++) {
        setImmediateHexColor(colors[i]);
        delay(500);
    }
    setImmediateHexColor(0x000000); // End off
}
