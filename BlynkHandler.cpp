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

void BlynkHandler::sendData(float pm1_0, float pm2_5, float pm10_0, float temperature, float humidity) {
    Blynk.beginGroup();
    
    Blynk.virtualWrite(V0, pm1_0);
    Blynk.virtualWrite(V1, pm2_5);
    Blynk.virtualWrite(V2, pm10_0);
    Blynk.virtualWrite(V4, temperature);
    Blynk.virtualWrite(V5, humidity);

    Blynk.endGroup();
}
