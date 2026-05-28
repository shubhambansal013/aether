#line 1 "/home/runner/work/aether/aether/DHTSensor.cpp"
#include "DHTSensor.h"

DHTSensor::DHTSensor(int pin) : _dht(pin, DHT22) {}

void DHTSensor::setup() {
    _dht.begin();
}

float DHTSensor::getTemperature() {
    float t = _dht.readTemperature();
    return isnan(t) ? INVALID_VALUE : t;
}

float DHTSensor::getHumidity() {
    float h = _dht.readHumidity();
    return isnan(h) ? INVALID_VALUE : h;
}
