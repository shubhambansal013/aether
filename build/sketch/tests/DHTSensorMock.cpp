#line 1 "/home/runner/work/aether/aether/tests/DHTSensorMock.cpp"
#include "DHTSensorMock.h"

DHTSensor::DHTSensor(int p) {}
void DHTSensor::setup() {}
float DHTSensor::getTemperature() { return mock_temp; }
float DHTSensor::getHumidity() { return mock_hum; }
