#ifndef DHTSENSOR_MOCK_H
#define DHTSENSOR_MOCK_H

#include "Arduino.h"

class DHTSensor {
public:
    DHTSensor(int pin);
    void setup();
    float getTemperature();
    float getHumidity();

    float mock_temp = 25.0;
    float mock_hum = 50.0;
};

#endif
