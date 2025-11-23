#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H

#include <Arduino.h>
#include <ArduinoOTA.h>

/*
  OTAHandler
  - Encapsulates ArduinoOTA initialization and handling for ESP8266 devices.
  - Usage:
      OTAHandler ota;
      ota.begin("my-password");   // after Wi-Fi is connected
      ...
      ota.handle();               // call regularly in loop()
*/

class OTAHandler {
public:
    OTAHandler();
    // Initialize OTA. Should be called AFTER Wi-Fi is connected.
    // password: optional OTA password (nullptr or empty to disable auth)
    // baseHostname: optional base for the hostname; final hostname will be baseHostname-XXXXXX where XXXXXX is chip id
    void begin(const char* password = nullptr, const char* baseHostname = "airmon");
    // Must be called frequently from loop to service OTA
    void handle();
    // Returns last reported progress percent (0-100)
    uint8_t lastProgress() const { return _lastProgress; }

private:
    bool _initialized;
    uint8_t _lastProgress;
    char _hostname[32];
    const char* _password;
    static OTAHandler* _instance; // used so lambdas can update the instance
};

#endif // OTA_HANDLER_H
