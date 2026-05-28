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
#include "OTAHandler.h"

class SystemController {
public:
    SystemController(PMSensor& pm, DHTSensor& dht, OLEDDisplay& oled,
                     RGBLEDHandler& led, WiFiHandler& wifi, BlynkHandler& blynk,
                     ButtonHandler& button);

    void setup();
    void update();

    void toggleStealthMode();
    void cycleSystemMode();

    /**
     * @brief Returns the current system data.
     * @return Const reference to SystemData.
     */
    const SystemData& getData() const { return _data; }

    /**
     * @brief Checks if the system is in Stealth Mode (Muted).
     * @return true if muted, false otherwise.
     */
    bool isMuted() const { return _isMuted; }

private:
    /**
     * @brief Processes user inputs (buttons).
     */
    void processInputs();

    /**
     * @brief Updates sensor data (PMS, DHT).
     */
    void updateSensors();

    /**
     * @brief Manages output components (OLED, LED, Blynk).
     */
    void updateOutputs();

    /**
     * @brief Manages the PM sensor state and reading.
     */
    void handlePMSensor();

    /**
     * @brief Logic for PM sensor in ACTIVE mode.
     */
    void handleActiveMode();

    /**
     * @brief Logic for PM sensor in PASSIVE mode (Wake/Sleep cycle).
     * @param now Current timestamp in millis.
     * @param elapsed Time elapsed since last state transition.
     */
    void handlePassiveMode(unsigned long now, unsigned long elapsed);

    /**
     * @brief Updates the OLED display data and triggers a refresh.
     */
    void updateUI();

    /**
     * @brief Transmits data to Blynk if conditions are met.
     */
    void handleBlynkTransmission();

    /**
     * @brief Loads system settings from EEPROM.
     */
    void loadSettings();

    /**
     * @brief Saves system settings to EEPROM.
     */
    void saveSettings();

    SystemData _data;
    PMSensor& _pmSensor;
    DHTSensor& _dhtSensor;
    OLEDDisplay& _oled;
    RGBLEDHandler& _led;
    WiFiHandler& _wifi;
    BlynkHandler& _blynk;
    ButtonHandler& _button;
    OTAHandler _ota;

    unsigned long _lastBlynk = 0;
    unsigned long _lastDHTRead = 0;
    const unsigned long DHT_READ_INTERVAL = 2000;
    unsigned long _stateTimer = 0;
    unsigned long _blynkFlashTimer = 0;
    bool _sensorAwake = true;
    bool _isDataFresh = false;
    bool _isMuted = false;
};

#endif
