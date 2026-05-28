#line 1 "/home/runner/work/aether/aether/BlynkHandler.h"
// BlynkHandler.h

#ifndef BLYNK_HANDLER_H
#define BLYNK_HANDLER_H

#include <Arduino.h>

// Forward declarations for libraries used in the .cpp file
#include <ESP8266WiFi.h>

class BlynkHandler {
public:
    BlynkHandler();
    // Updated 'begin' takes only WiFi credentials
    void begin(const char* ssid, const char* pass);
    
    // 'run' is simplified since we no longer maintain a constant Blynk connection
    void run();

    // Updated 'sendData' now requires the Blynk Auth Token
    void sendData(const char* auth, float pm1_0, float pm2_5, float pm10_0, float temperature, float humidity);
    
private:
    // No longer need BlynkTimer or Blynk object for HTTP method.
};

#endif // BLYNK_HANDLER_H
