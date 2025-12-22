#include "DHTSensor.h"

// The pin is now passed from the constructor in pawan.ino
DHTSensor::DHTSensor(int pin) : dht(pin, DHTTYPE) {}

void DHTSensor::setup() {
    dht.begin();
    Serial.println("DHT22 Sensor initialized.");
}

float DHTSensor::readHumidity() {
    float h = dht.readHumidity();
    if (isnan(h)) {
        Serial.println("Failed to read humidity from DHT sensor!");
        return NAN;
    }
    return h;
}

float DHTSensor::readTemperature() {
    float t = dht.readTemperature();
    if (isnan(t)) {
        Serial.println("Failed to read temperature from DHT sensor!");
        return NAN;
    }
    return t;
}
