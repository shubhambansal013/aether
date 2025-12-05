#ifndef DHTSENSOR_H
#define DHTSENSOR_H

#include "pins.h"
#include "DHT.h"

// Define the DHT Sensor Pin and Type
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

class DHTSensor {
public:
    DHTSensor();
    void setup();
    float readHumidity();
    float readTemperature();

private:
    DHT dht;
};

#endif