#include "PMSensor.h"

PMSensor::PMSensor(int rxPin, int setPin) 
    : _pmSerial(rxPin, -1), _setPin(setPin) {}

void PMSensor::begin(long baudRate) {
    pinMode(_setPin, OUTPUT);
    digitalWrite(_setPin, HIGH); // Start in awake state
    _pmSerial.begin(baudRate);
}

void PMSensor::sleep() {
    digitalWrite(_setPin, LOW);
}

void PMSensor::wakeup() {
    digitalWrite(_setPin, HIGH);
}

void PMSensor::clearBuffer() {
    while(_pmSerial.available()) {
        _pmSerial.read();
    }
}

bool PMSensor::readData(float& pm1_0, float& pm2_5, float& pm10_0) {
    // In Active Mode (default), sensor sends 32-byte packets
    if (_pmSerial.available() < 32) return false;
    
    byte buffer[32];
    // Find the start of the frame (0x42 0x4D)
    while (_pmSerial.available() >= 32) {
        if (_pmSerial.read() != 0x42) continue; 
        if (_pmSerial.read() != 0x4D) continue; 
        
        _pmSerial.readBytes(&buffer[2], 30);
        
        // Byte 10-11: PM1.0, 12-13: PM2.5, 14-15: PM10 (Standard Units)
        pm1_0  = (buffer[10] << 8) | buffer[11]; 
        pm2_5  = (buffer[12] << 8) | buffer[13]; 
        pm10_0 = (buffer[14] << 8) | buffer[15]; 
        
        return true;
    }
    return false;
}
