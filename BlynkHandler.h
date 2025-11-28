// BlynkHandler.h
#ifndef BLYNK_HANDLER_H
#define BLYNK_HANDLER_H

#include "blynk_config.h"

// Forward declaration
class BlynkTimer;

class BlynkHandler {
public:
    BlynkHandler(); // Constructor
    void begin(const char* auth, const char* ssid, const char* pass);
    void run();
    void sendSensorData(float pm1_0, float pm2_5, float pm10_0);
    void sendFirmwareVersion(const char* version);

private:
    BlynkTimer* _timer; // Pointer to the timer
};

#endif // BLYNK_HANDLER_H
