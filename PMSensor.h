// PMSensor.h

#ifndef PMSENSOR_H
#define PMSENSOR_H

#include <Arduino.h>
#include <SoftwareSerial.h>

// Forward declaration
struct pms5003data;

class PMSensor {
public:
    /**
     * @brief Constructor.
     * @param rxPin RX pin for the sensor (connects to sensor TX).
     * @param txPin TX pin for the sensor (connects to sensor RX).
     */
    PMSensor(int rxPin, int txPin);

    /**
     * @brief Initializes SoftwareSerial communication.
     */
    void begin(long baudRate);

    /**
     * @brief Reads data from the sensor (Mock or Real).
     * @param pm1_0 Reference to update PM 1.0 variable.
     * @param pm2_5 Reference to update PM 2.5 variable.
     * @param pm10_0 Reference to update PM 10.0 variable.
     * @param useMockData If true, generates random data; otherwise, reads serial.
     * @return true if data was successfully obtained, false otherwise.
     */
    bool readData(float& pm1_0, float& pm2_5, float& pm10_0, bool useMockData);

    // --- UART Command Functions ---
    /**
     * @brief Sends command to switch sensor to passive output mode.
     * In this mode, the MCU must request data using requestData().
     */
    void switchToPassiveMode();

    /**
     * @brief Sends command to switch sensor to auto output mode.
     * In this mode, the sensor automatically sends data every 1 second.
     */
    void switchToAutoMode();

    /**
     * @brief Sends command to enter standby (sleep) mode.
     * Turns off fan and laser for power saving.
     */
    void enterStandbyMode();

    /**
     * @brief Sends command to enter normal working mode (wakes up).
     * Note: Takes >30s for stable readings after waking.
     */
    void enterNormalMode();
    
    /**
     * @brief Sends command to request data in passive mode.
     */
    void requestData();

private:
    SoftwareSerial _pmSerial;
    pms5003data* _data;
    
    /**
     * @brief Helper function to calculate and send a generic 7-byte command.
     * @param cmd Command byte (0xE1, 0xE2, or 0xE4).
     * @param dataL Data low byte (0x00 or 0x01).
     */
    void sendCommand(byte cmd, byte dataL);

    bool readPmsData();
    bool readSensorPacket();
    void generateMockData(float& pm1_0, float& pm2_5, float& pm10_0);
};

#endif
