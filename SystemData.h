#ifndef SYSTEMDATA_H
#define SYSTEMDATA_H

#include <Arduino.h>

struct SystemData {
    // Sensor Values
    float pm1_0 = 0;
    float pm2_5 = 0;
    float pm10_0 = 0;
    float temp = 0;
    float hum = 0;

    // System States
    String wifiStatus = "Unknown";
    bool isWarmup = true;
    bool isSleeping = false;
    bool showBlynkIcon = false;
    int countdown = 0;
};

#endif
