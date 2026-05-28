#ifndef PMSENSOR_MOCK_H
#define PMSENSOR_MOCK_H

#include "Arduino.h"
#include "SoftwareSerial.h"

class PMSensor {
public:
    PMSensor(int rx, int tx);
    void begin(long baud);
    void sleep();
    void wakeup();
    void clearBuffer();
    bool readData(float& pm1, float& pm2, float& pm10);

    // Mock control/spy
    bool mock_readData_return = true;
    float mock_pm1 = 1.0, mock_pm2 = 2.0, mock_pm10 = 3.0;
    int wakeupCount = 0;
    int sleepCount = 0;

private:
    SoftwareSerial _pmSerial;
    int _setPin;
};

#endif
