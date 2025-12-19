#include "PMSensor.h"

PMSensor::PMSensor(int rxPin, int setPin) 
    : _pmSerial(rxPin, -1), _setPin(setPin) {}

void PMSensor::begin(long baudRate) {
    pinMode(_setPin, OUTPUT);
    digitalWrite(_setPin, HIGH); // Default to ON
    _pmSerial.begin(baudRate);
}

void PMSensor::sleep() { digitalWrite(_setPin, LOW); }

void PMSensor::wakeup() { digitalWrite(_setPin, HIGH); }

void PMSensor::clearBuffer() {
    while(_pmSerial.available()) { _pmSerial.read(); }
}

/**
 * @brief Public method to handle the full reading cycle.
 */
bool PMSensor::readData(float& pm1_0, float& pm2_5, float& pm10_0) {
    // 1. Ensure enough data is present for a full 32-byte packet
    if (_pmSerial.available() < 32) return false;
    
    byte buffer[32];
    
    // 2. Sync with the start bytes (0x42 0x4D)
    if (!findHeader()) return false;
    
    // 3. Populate buffer with the header we just verified
    buffer[0] = 0x42;
    buffer[1] = 0x4D;
    
    // 4. Read the remaining 30 bytes of the packet
    _pmSerial.readBytes(&buffer[2], 30);
    
    // 5. Verify data integrity via Checksum
    if (!isValidChecksum(buffer, 32)) {
        Serial.println(">> PMS ERROR: Checksum Mismatch. Discarding packet.");
        return false;
    }
    
    // 6. Parse the data from the buffer
    parseBuffer(buffer, pm1_0, pm2_5, pm10_0);
    
    return true;
}

// ----------------------------------------------------------------------
// 🛠️ PRIVATE HELPER METHODS
// ----------------------------------------------------------------------

/**
 * @brief Searches the serial stream for the 0x42 0x4D start sequence.
 */
bool PMSensor::findHeader() {
    while (_pmSerial.available() >= 32) {
        if (_pmSerial.read() == 0x42) {
            if (_pmSerial.peek() == 0x4D) {
                _pmSerial.read(); // Consume the 0x4D
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief Calculates and verifies the checksum for the 32-byte packet.
 */
bool PMSensor::isValidChecksum(byte* buffer, int length) {
    uint16_t calculatedSum = 0;
    // Sum bytes 0 through 29
    for (int i = 0; i < 30; i++) {
        calculatedSum += buffer[i];
    }
    
    uint16_t sentChecksum = (buffer[30] << 8) | buffer[31];
    return (calculatedSum == sentChecksum);
}

/**
 * @brief Extracts specific PM values from the validated buffer.
 */
void PMSensor::parseBuffer(byte* buffer, float& pm1_0, float& pm2_5, float& pm10_0) {
    // The sensor sends data in Big Endian (High byte followed by Low byte)
    // Indices 10-15 correspond to "Standard Particulate Matter" (ug/m3)
    pm1_0  = (buffer[10] << 8) | buffer[11]; 
    pm2_5  = (buffer[12] << 8) | buffer[13]; 
    pm10_0 = (buffer[14] << 8) | buffer[15]; 
}
