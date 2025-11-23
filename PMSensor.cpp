// PMSensor.cpp

#include "PMSensor.h"

PMSensor::PMSensor(int rxPin, int txPin) 
    : _pmSerial(rxPin, txPin) {
    // Seed the random number generator using analogRead of an unconnected pin (for better randomness)
    randomSeed(analogRead(0));
}

void PMSensor::begin(long baudRate) {
    _pmSerial.begin(baudRate);
    Serial.println("PMSA003/MPM11 Sensor Serial Initialized.");
}

// --- Mock Data Generator (For Testing) ---

void PMSensor::generateMockData(float& pm1_0, float& pm2_5, float& pm10_0) {
    // Generates realistic, increasing PM values
    
    // PM 2.5 (Most critical value, base the others on it)
    pm2_5 = (float)random(10, 150); 
    
    // PM 1.0 is generally lower than PM 2.5
    pm1_0 = pm2_5 - (float)random(2, 10);
    if (pm1_0 < 0) pm1_0 = 1.0;

    // PM 10.0 is generally higher than PM 2.5
    pm10_0 = pm2_5 + (float)random(5, 20);

    // Apply a little smoothing (optional, but makes mock data look better)
    pm1_0 = round(pm1_0 * 10.0) / 10.0;
    pm2_5 = round(pm2_5 * 10.0) / 10.0;
    pm10_0 = round(pm10_0 * 10.0) / 10.0;
}

// --- Real Sensor Reading (Placeholder for later) ---

bool PMSensor::readSensorSerial() {
    // ⚠️ IMPLEMENTATION FOR REAL SENSOR READING GOES HERE LATER
    // For now, it always returns false to force mock data usage.
    while (_pmSerial.available()) _pmSerial.read(); // Clear buffer
    return false;
}

// --- Main Read Function ---

bool PMSensor::readData(float& pm1_0, float& pm2_5, float& pm10_0, bool useMockData) {
    if (useMockData) {
        generateMockData(pm1_0, pm2_5, pm10_0);
        return true;
    } else {
        // This will attempt to read the real sensor (currently returns false)
        if (readSensorSerial()) {
            // Later: add checksum and parsing logic here
            return true;
        }
        return false;
    }
}