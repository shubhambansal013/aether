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

private:
    SoftwareSerial _pmSerial;
    pms5003data* _data;

    bool readPmsData();
    
    // Generates random numbers for testing
    void generateMockData(float& pm1_0, float& pm2_5, float& pm10_0);
};

#endif
