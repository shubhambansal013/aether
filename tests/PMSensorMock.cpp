#include "PMSensorMock.h"

PMSensor::PMSensor(int r, int s) : _pmSerial(r, s), _setPin(s) {}
void PMSensor::begin(long b) {}
void PMSensor::sleep() { sleepCount++; }
void PMSensor::wakeup() { wakeupCount++; }
void PMSensor::clearBuffer() {}
bool PMSensor::readData(float& pm1, float& pm2, float& pm10) {
    pm1 = mock_pm1;
    pm2 = mock_pm2;
    pm10 = mock_pm10;
    return mock_readData_return;
}
