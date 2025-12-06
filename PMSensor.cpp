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

// bool PMSensor::readPmsData() {
//     // 1. Synchronize: Wait for the two-byte START_SEQUENCE (0x42 0x4D)
//     Serial.print("PMSensor available: "); Serial.println(_pmSerial.available());
//     while (_pmSerial.available()) {
//         byte incomingByte = _pmSerial.read();

//         if (incomingByte == 0x42) { // START_BYTE_1
//             Serial.println("Found the first start byte. Check if the second byte is available and correct.");
//             if (_pmSerial.available() < 31) { // PACKET_SIZE - 1
//                 Serial.println("Not enough data immediately available, wait a moment");
//                 delay(5);
//                 if (_pmSerial.available() < 31) {
//                     Serial.println("If still not enough, assume misalignment and discard this byte");
//                     continue; 
//                 }
//             }

//             if (_pmSerial.peek() == 0x4D) { // START_BYTE_2
//                 // --- Synchronization Found ---
//                 uint8_t buffer[32];
//                 buffer[0] = incomingByte;
//                 buffer[1] = _pmSerial.read(); 

//                 // 2. Read the rest of the fixed-size packet (30 bytes remaining)
//                 int bytesRead = _pmSerial.readBytes(&buffer[2], 30);

//                 if (bytesRead < 30) {
//                     Serial.println("Error: Timeout while waiting for full packet.");
//                     return false;
//                 }

//                 // 3. Checksum Validation
//                 unsigned int calculatedChecksum = 0;
//                 for (int i = 0; i < 30; i++) {
//                     calculatedChecksum += buffer[i];
//                 }
                
//                 unsigned int receivedChecksum = makeWord(buffer[30], buffer[31]);

//                 if (calculatedChecksum != receivedChecksum) {
//                     Serial.print("Error: Checksum mismatch. Calculated: ");
//                     Serial.print(calculatedChecksum);
//                     Serial.print(", Received: ");
//                     Serial.println(receivedChecksum);
//                     return false;
//                 }

//                 // 4. Data Parsing (from the validated buffer)
//                 _data->framelen = makeWord(buffer[2], buffer[3]);
//                 _data->pm10_standard = makeWord(buffer[4], buffer[5]);
//                 _data->pm25_standard = makeWord(buffer[6], buffer[7]);
//                 _data->pm100_standard = makeWord(buffer[8], buffer[9]);
//                 _data->pm10_env = makeWord(buffer[10], buffer[11]);
//                 _data->pm25_env = makeWord(buffer[12], buffer[13]);
//                 _data->pm100_env = makeWord(buffer[14], buffer[15]);
//                 _data->particles_03um = makeWord(buffer[16], buffer[17]);
//                 _data->particles_05um = makeWord(buffer[18], buffer[19]);
//                 _data->particles_10um = makeWord(buffer[20], buffer[21]);
//                 _data->particles_25um = makeWord(buffer[22], buffer[23]);
//                 _data->particles_50um = makeWord(buffer[24], buffer[25]);
//                 _data->particles_100um = makeWord(buffer[26], buffer[27]);
//                 _data->unused = makeWord(buffer[28], buffer[29]);
//                 _data->checksum = receivedChecksum;
                
//                 return true; // Packet successfully read and parsed

//             }
//         }
//     }
//     return false; // No data available or sync failed
// }

bool PMSensor::readPmsData() {
    return readSensorPacket();
}


// Function to read and process the sensor data packet
bool readSensorPacket() {
  // 1. Synchronize: Wait for the two-byte START_SEQUENCE (0x42 0x4D)
  Serial.print("Sensor available: "); Serial.println(_pmSerial.available());
  while (_pmSerial.available()) {
    byte incomingByte = _pmSerial.read();

    if (incomingByte == START_BYTE_1) {
      Serial.println("Found the first start byte. Check if the second byte is available and correct.");
      if (mpmSensor.available() < (PACKET_SIZE - 1)) {
        Serial.println("Not enough data immediately available, wait a moment");
        delay(5);
        if (_pmSerial.available() < (PACKET_SIZE - 1)) {
          Serial.println("If still not enough, assume misalignment and discard this byte");
          continue;
        }
      }

      if (mpmSensor.peek() == START_BYTE_2) {
        // --- Synchronization Found ---
        
        // Read the second start byte immediately
        dataPacket[0] = incomingByte;
        dataPacket[1] = _pmSerial.read();

        // 2. Read the rest of the fixed-size packet (30 bytes remaining)
        int bytesToRead = PACKET_SIZE - 2;
        long timeout = millis();

        while (_pmSerial.available() < bytesToRead) {
          if (millis() - timeout > 100) {
            Serial.println("Error: Timeout while waiting for full packet.");
            return false;
          }
          delay(1);
        }

        // Read the remaining bytes (from index 2 to 31)
        for (int i = 2; i < PACKET_SIZE; i++) {
          dataPacket[i] = _pmSerial.read();
        }

        // 3. Checksum Validation (Sum of all bytes up to index 29)
        unsigned int calculatedChecksum = 0;
        for (int i = 0; i < PACKET_SIZE - 2; i++) {
          calculatedChecksum += dataPacket[i];
        }

        // The received checksum is stored in the last two bytes (index 30 and 31, MSB first)
        unsigned int receivedChecksum = (dataPacket[PACKET_SIZE - 2] << 8) | dataPacket[PACKET_SIZE - 1];

        if (calculatedChecksum != receivedChecksum) {
          Serial.print("Error: Checksum mismatch. Calculated: ");
          Serial.print(calculatedChecksum);
          Serial.print(", Received: ");
          Serial.println(receivedChecksum);
          return false;
        }

        // 4. Data Parsing: Extract the PM values (Big-Endian/MSB first)
        
        // Standard Particulate Matter Concentrations (Industrial)
        // PM1.0 (Standard) - Bytes 4 and 5
        unsigned int pm1_0_standard = (dataPacket[4] << 8) | dataPacket[5];
        // PM2.5 (Standard) - Bytes 6 and 7
        unsigned int pm2_5_standard = (dataPacket[6] << 8) | dataPacket[7];
        // PM10 (Standard) - Bytes 8 and 9
        unsigned int pm10_standard = (dataPacket[8] << 8) | dataPacket[9];

        // Atmospheric Environment Concentrations (Ambient)
        // PM1.0 (Atmospheric) - Bytes 10 and 11
        unsigned int pm1_0_atm = (dataPacket[10] << 8) | dataPacket[11];
        // PM2.5 (Atmospheric) - Bytes 12 and 13
        unsigned int pm2_5_atm = (dataPacket[12] << 8) | dataPacket[13];
        // PM10 (Atmospheric) - Bytes 14 and 15
        unsigned int pm10_atm = (dataPacket[14] << 8) | dataPacket[15];
        
        // 5. Output Results ---
        Serial.println("----------------------------------------");
        Serial.print("Raw Hex Packet: ");
        for (int i = 0; i < PACKET_SIZE; i++) {
          if (dataPacket[i] < 0x10) Serial.print("0");
          Serial.print(dataPacket[i], HEX);
          Serial.print(" ");
        }
        Serial.println();
        Serial.println("--- Parsed PM Data (ug/m^3) ---");
        
        Serial.println("Standard Particulate Matter (Industrial):");
        Serial.print("PM 1.0 (Standard): "); Serial.println(pm1_0_standard);
        Serial.print("PM 2.5 (Standard): "); Serial.println(pm2_5_standard);
        Serial.print("PM 10.0 (Standard): "); Serial.println(pm10_standard);
        
        Serial.println("Atmospheric Environment (Ambient):");
        Serial.print("PM 1.0 (Atmospheric): "); Serial.println(pm1_0_atm);
        Serial.print("PM 2.5 (Atmospheric): "); Serial.println(pm2_5_atm);
        Serial.print("PM 10.0 (Atmospheric): "); Serial.println(pm10_atm);

        Serial.println("----------------------------------------");

        _data->pm10_standard = pm1_0_standard;
        _data->pm25_standard = pm2_5_standard;
        _data->pm100_standard = pm10_standard;
        _data->pm10_env = pm1_0_atm;
        _data->pm25_env = pm2_5_atm;
        _data->pm100_env = pm10_atm;
        _data->checksum = receivedChecksum;
        return true; // Packet successfully read and parsed

      } else {
          Serial.println("Byte 1 was 0x42, but Byte 2 was not 0x4D. Discard 0x42 and continue search.");
      }
    }
  }
  return false; // No data available
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
