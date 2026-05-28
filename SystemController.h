#ifndef SYSTEM_CONTROLLER_H
#define SYSTEM_CONTROLLER_H

#include <Arduino.h>
#include "Config.h"
#include "SystemData.h"
#include "PMSensor.h"
#include "DHTSensor.h"
#include "OLEDDisplay.h"
#include "RGBLEDHandler.h"
#include "WiFiHandler.h"
#include "BlynkHandler.h"
#include "ButtonHandler.h"

class SystemController {
public:
    SystemController(PMSensor& pm, DHTSensor& dht, OLEDDisplay& oled,
                     RGBLEDHandler& led, WiFiHandler& wifi, BlynkHandler& blynk,
                     ButtonHandler& button);

    void setup();
    void update();

    void toggleStealthMode();
    void cycleSystemMode();

    // Getters for testing/UI
    const SystemData& getData() const { return _data; }
    bool isMuted() const { return _isMuted; }

private:
    void handlePMSensor();
    void updateUI();
    void handleBlynkTransmission();
    void loadSettings();
    void saveSettings();

    SystemData _data;
    PMSensor& _pmSensor;
    DHTSensor& _dhtSensor;
    OLEDDisplay& _oled;
    RGBLEDHandler& _led;
    WiFiHandler& _wifi;
    BlynkHandler& _blynk;
    ButtonHandler& _button;

    unsigned long _lastBlynk = 0;
    unsigned long _stateTimer = 0;
    unsigned long _blynkFlashTimer = 0;
    bool _sensorAwake = true;
    bool _isDataFresh = false;
    bool _isMuted = false;
};

#endif
