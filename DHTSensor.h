#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include "DHT.h"

class DHTSensor {
public:
    DHTSensor(int pin);
    void setup();
    float getTemperature(); // Returns -999 on error
    float getHumidity();    // Returns -999 on error
    
    // Static constant for error checking elsewhere
    static constexpr float INVALID_VALUE = -999.0;

private:
    DHT _dht;
};

#endif
