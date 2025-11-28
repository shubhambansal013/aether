#ifndef DHTSENSOR_H
#define DHTSENSOR_H

#include "DHT.h"

// Define the DHT Sensor Pin and Type
#define DHTPIN D4 // GIO2
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