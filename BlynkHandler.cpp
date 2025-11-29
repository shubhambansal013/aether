// BlynkHandler.cpp
#include "BlynkHandler.h"
#include <BlynkSimpleEsp8266.h>

BlynkHandler::BlynkHandler() {
    _timer = new BlynkTimer();
}

void BlynkHandler::begin(const char* auth, const char* ssid, const char* pass) {
    Blynk.begin(auth, ssid, pass);
}

void BlynkHandler::run() {
    Blynk.run();
    _timer->run();
}

void BlynkHandler::sendSensorData(float pm1_0, float pm2_5, float pm10_0) {
    Blynk.virtualWrite(V0, pm1_0);
    Blynk.virtualWrite(V1, pm2_5);
    Blynk.virtualWrite(V2, pm10_0);
}

void BlynkHandler::sendTemperatureHumidity(float temperature, float humidity) {
    Blynk.virtualWrite(V4, temperature);
    Blynk.virtualWrite(V5, humidity);
}

void BlynkHandler::sendFirmwareVersion(const char* version) {
    Blynk.virtualWrite(V3, version);
}
