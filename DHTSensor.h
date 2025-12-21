#ifndef DHTSENSOR_H
#define DHTSENSOR_H

#include <Arduino.h>
#include "DHT.h"

// Note: DHTTYPE is still defined here as it's specific to the sensor hardware
#define DHTTYPE DHT22   

class DHTSensor {
public:
    // Constructor now takes the pin
    DHTSensor(int pin);
    
    void setup();
    float readHumidity();
    float readTemperature();

private:
    DHT dht;
};

#endif
