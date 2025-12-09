// PMSensor.cpp

#include "PMSensor.h"

// PM Sensor Protocol Constants 
const byte START_BYTE_1 = 0x42;
const byte START_BYTE_2 = 0x4D;
const byte PACKET_SIZE = 32;   // Total bytes in the data packet
const byte COMMAND_SIZE = 7;   // Total bytes in a command frame

// Buffer to hold the incoming data packet
byte dataPacket[PACKET_SIZE];

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


// --- Private Command Helper ---

/**
 * The command format is 7 bytes: 
 * 0x42 0x4D CMD 0x00 DATAL SUMH SUML
 */
void PMSensor::sendCommand(byte cmd, byte dataL) {
    // Note: Data byte 1 (DATAH) is always 0x00 for these specific commands.
    byte commandFrame[COMMAND_SIZE] = {
        START_BYTE_1,   // 0: Frame Header 1 (0x42)
        START_BYTE_2,   // 1: Frame Header 2 (0x4D)
        cmd,            // 2: Command byte (0xE1, 0xE2, or 0xE4)
        0x00,           // 3: Data byte 1 (DATAH)
        dataL,          // 4: Data byte 2 (DATAL)
        0x00,           // 5: Check byte High (Placeholder)
        0x00            // 6: Check byte Low (Placeholder)
    };

    // 1. Calculate the 16-bit checksum (sum of bytes 0 through 4)
    uint16_t checksum = 0;
    for (int i = 0; i < 5; i++) {
        checksum += commandFrame[i];
    }

    // 2. Insert the calculated checksum into the command frame (MSB first)
    commandFrame[5] = (byte)(checksum >> 8);  // Check byte High
    commandFrame[6] = (byte)(checksum & 0xFF); // Check byte Low

    // 3. Send the command
    _pmSerial.write(commandFrame, COMMAND_SIZE);
    _pmSerial.flush(); // Ensure all bytes are sent
    
    Serial.print("Command Sent (0x"); 
    Serial.print(cmd, HEX); 
    Serial.print(" 0x"); 
    Serial.print(dataL, HEX);
    Serial.print("). Checksum: 0x"); 
    Serial.println(checksum, HEX);
}


// --- Public Command Functions ---

void PMSensor::switchToPassiveMode() {
    sendCommand(0xE1, 0x00); 
}

void PMSensor::switchToAutoMode() {
    sendCommand(0xE1, 0x01); 
}

void PMSensor::enterStandbyMode() {
    sendCommand(0xE4, 0x00);
}

void PMSensor::enterNormalMode() {
    sendCommand(0xE4, 0x01);
}

void PMSensor::requestData() {
    // 0xE2 0x00 is the command to request a 32-byte packet in passive mode.
    sendCommand(0xE2, 0x00);
}


// --- Real Sensor Reading ---

bool PMSensor::readSensorPacket() {
  // 1. Synchronize: Wait for the two-byte START_SEQUENCE (0x42 0x4D)
  Serial.print("Sensor available: "); Serial.println(_pmSerial.available());
  while (_pmSerial.available()) {
    byte incomingByte = _pmSerial.read();

    if (incomingByte == START_BYTE_1) {
      Serial.println("Found the first start byte. Check if the second byte is available and correct.");
      if (_pmSerial.available() < (PACKET_SIZE - 1)) {
        Serial.println("Not enough data immediately available, wait a moment");
        delay(5);
        if (_pmSerial.available() < (PACKET_SIZE - 1)) {
          Serial.println("If still not enough, assume misalignment and discard this byte");
          continue;
        }
      }

      if (_pmSerial.peek() == START_BYTE_2) {
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
        Serial.
