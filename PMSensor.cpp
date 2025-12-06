// PMSensor.cpp

#include "PMSensor.h"

// Struct to hold PMS5003 data
struct pms5003data {
    uint16_t framelen;
    uint16_t pm10_standard, pm25_standard, pm100_standard;
    uint16_t pm10_env, pm25_env, pm100_env;
    uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
    uint16_t unused;
    uint16_t checksum;
};

PMSensor::PMSensor(int rxPin, int txPin) 
    : _pmSerial(rxPin, txPin) {
    _data = new pms5003data;
    // Seed the random number generator
    randomSeed(analogRead(0));
}

void PMSensor::begin(long baudRate) {
    _pmSerial.begin(baudRate);
    Serial.println("PMS5003 Sensor Serial Initialized.");
}

// --- Mock Data Generator (For Testing) ---

void PMSensor::generateMockData(float& pm1_0, float& pm2_5, float& pm10_0) {
    pm2_5 = (float)random(10, 150); 
    pm1_0 = pm2_5 - (float)random(2, 10);
    if (pm1_0 < 0) pm1_0 = 1.0;
    pm10_0 = pm2_5 + (float)random(5, 20);
    pm1_0 = round(pm1_0 * 10.0) / 10.0;
    pm2_5 = round(pm2_5 * 10.0) / 10.0;
    pm10_0 = round(pm10_0 * 10.0) / 10.0;
}

// --- Real Sensor Reading ---

bool PMSensor::readPmsData() {
    // 1. Synchronize: Wait for the two-byte START_SEQUENCE (0x42 0x4D)
    Serial.print("PMSensor available: "); Serial.println(_pmSerial.available());
    while (_pmSerial.available()) {
        byte incomingByte = _pmSerial.read();

        if (incomingByte == 0x42) { // START_BYTE_1
            // Found the first start byte. Check if the second byte is available and correct.
            if (_pmSerial.available() < 31) { // PACKET_SIZE - 1
                // Not enough data immediately available, wait a moment
                delay(5);
                if (_pmSerial.available() < 31) {
                    // If still not enough, assume misalignment and discard this byte
                    continue; 
                }
            }

            if (_pmSerial.peek() == 0x4D) { // START_BYTE_2
                // --- Synchronization Found ---
                uint8_t buffer[32];
                buffer[0] = incomingByte;
                buffer[1] = _pmSerial.read(); 

                // 2. Read the rest of the fixed-size packet (30 bytes remaining)
                int bytesRead = _pmSerial.readBytes(&buffer[2], 30);

                if (bytesRead < 30) {
                    Serial.println("Error: Timeout while waiting for full packet.");
                    return false;
                }

                // 3. Checksum Validation
                unsigned int calculatedChecksum = 0;
                for (int i = 0; i < 30; i++) {
                    calculatedChecksum += buffer[i];
                }
                
                unsigned int receivedChecksum = makeWord(buffer[30], buffer[31]);

                if (calculatedChecksum != receivedChecksum) {
                    Serial.print("Error: Checksum mismatch. Calculated: ");
                    Serial.print(calculatedChecksum);
                    Serial.print(", Received: ");
                    Serial.println(receivedChecksum);
                    return false;
                }

                // 4. Data Parsing (from the validated buffer)
                _data->framelen = makeWord(buffer[2], buffer[3]);
                _data->pm10_standard = makeWord(buffer[4], buffer[5]);
                _data->pm25_standard = makeWord(buffer[6], buffer[7]);
                _data->pm100_standard = makeWord(buffer[8], buffer[9]);
                _data->pm10_env = makeWord(buffer[10], buffer[11]);
                _data->pm25_env = makeWord(buffer[12], buffer[13]);
                _data->pm100_env = makeWord(buffer[14], buffer[15]);
                _data->particles_03um = makeWord(buffer[16], buffer[17]);
                _data->particles_05um = makeWord(buffer[18], buffer[19]);
                _data->particles_10um = makeWord(buffer[20], buffer[21]);
                _data->particles_25um = makeWord(buffer[22], buffer[23]);
                _data->particles_50um = makeWord(buffer[24], buffer[25]);
                _data->particles_100um = makeWord(buffer[26], buffer[27]);
                _data->unused = makeWord(buffer[28], buffer[29]);
                _data->checksum = receivedChecksum;
                
                return true; // Packet successfully read and parsed

            }
        }
    }
    return false; // No data available or sync failed
}

// --- Main Read Function ---

bool PMSensor::readData(float& pm1_0, float& pm2_5, float& pm10_0, bool useMockData) {
    if (useMockData) {
        generateMockData(pm1_0, pm2_5, pm10_0);
        return true;
    } else {
        if (readPmsData()) {
            pm1_0 = _data->pm10_standard;
            pm2_5 = _data->pm25_standard;
            pm10_0 = _data->pm100_standard;
            return true;
        }
        return false;
    }
}
