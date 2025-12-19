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
     * @brief Reads data from the sensor stream.
     * @return true if a valid 32-byte packet was parsed.
     */
    bool readData(float& pm1_0, float& pm2_5, float& pm10_0);

private:
    SoftwareSerial _pmSerial;
    int _setPin;
};

#endif
