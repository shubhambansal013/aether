#ifndef PMSENSOR_H
#define PMSENSOR_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class PMSensor {
public:
    /**
     * @brief Constructor.
     * @param rxPin RX pin for the sensor (connects to sensor TX).
     * @param setPin SET pin for the sensor (Hardware Sleep/Wake).
     */
    PMSensor(int rxPin, int setPin);

    /**
     * @brief Initializes GPIOs and SoftwareSerial communication.
     */
    void begin(long baudRate = 9600);

    /**
     * @brief Hardware-level sleep (Pulls SET pin LOW).
     */
    void sleep();

    /**
     * @brief Hardware-level wakeup (Pulls SET pin HIGH).
     */
    void wakeup();

    /**
     * @brief Flushes the serial buffer to ensure fresh data.
     */
    void clearBuffer();

    /**
     * @brief Reads data from the sensor stream and verifies integrity.
     * @return true if a valid 32-byte packet was parsed and checksum passed.
     */
    bool readData(float& pm1_0, float& pm2_5, float& pm10_0);

private:
    SoftwareSerial _pmSerial;
    int _setPin;

    // --- Private Helper Methods (Matches the refactored .cpp) ---
    
    /**
     * @brief Searches the serial stream for the 0x42 0x4D start sequence.
     */
    bool findHeader();

    /**
     * @brief Calculates and verifies the checksum for the 32-byte packet.
     */
    bool isValidChecksum(byte* buffer, int length);

    /**
     * @brief Extracts specific PM values from the validated buffer.
     */
    void parseBuffer(byte* buffer, float& pm1_0, float& pm2_5, float& pm10_0);

    /**
     * @brief Syncs with header and reads the full 32-byte packet.
     * @return true if packet was successfully read.
     */
    bool readPacket(byte* buffer);
};

#endif
