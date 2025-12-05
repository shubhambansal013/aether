#include "DHTSensor.h"
#include <Arduino.h>

DHTSensor::DHTSensor() : dht(DHTPIN, DHTTYPE) {}

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