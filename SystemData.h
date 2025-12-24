#ifndef SYSTEMDATA_H
#define SYSTEMDATA_H

#include <Arduino.h>

enum SystemMode {
    MODE_AUTO,    // Default: Active -> Passive cycle
    MODE_ACTIVE,  // Continuous Wake
    MODE_PASSIVE  // Wake/Sleep only (No long active phase)
};

struct SystemData {
    float pm1_0 = -1.0, pm2_5 = -1.0, pm10_0 = -1.0;
    float temp = -999.0, hum = -999.0;
    
    SystemMode currentMode = MODE_AUTO; // Track 3 modes
    String wifiStatus = "Unknown";
    bool isWarmup = true;
    bool isSleeping = false;
    bool showBlynkIcon = false;
    int countdown = 0;
};

#endif
