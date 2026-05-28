#include "SystemController.h"
#include <EEPROM.h>
#include "blynk_config.h"

SystemController::SystemController(PMSensor& pm, DHTSensor& dht, OLEDDisplay& oled,
                                   RGBLEDHandler& led, WiFiHandler& wifi,
                                   BlynkHandler& blynk, ButtonHandler& button)
    : _pmSensor(pm), _dhtSensor(dht), _oled(oled), _led(led),
      _wifi(wifi), _blynk(blynk), _button(button) {}

void SystemController::setup() {
    _button.setup();
    _oled.setup();
    _led.setup();

    loadSettings();

    if (!_isMuted) {
        _led.startupSequence();
    }

    _pmSensor.begin(9600);
    _dhtSensor.setup();
    _wifi.startConnect();
    _ota.setup(&_oled, &_led);

    _data.isWarmup = false;
    _sensorAwake = true;
    _pmSensor.wakeup();

    _stateTimer = millis();
}

void SystemController::update() {
    _wifi.handleConnect();
    _ota.handle();
    processInputs();
    updateSensors();
    updateOutputs();
}

void SystemController::processInputs() {
    if (_button.isPressed()) cycleSystemMode();
    if (_button.isLongPressed()) toggleStealthMode();
}

void SystemController::updateSensors() {
    handlePMSensor();
    _data.temp = _dhtSensor.getTemperature();
    _data.hum = _dhtSensor.getHumidity();
}

void SystemController::updateOutputs() {
    if (!_isMuted) {
        updateUI();
        _led.updateLED(_data.pm2_5);
    } else {
        _oled.clear();
        _led.turnOff();
    }
    handleBlynkTransmission();
}

void SystemController::toggleStealthMode() {
    _isMuted = !_isMuted;
    saveSettings();

    if (_isMuted) {
        _oled.clear();
        _led.turnOff();
        Serial.println(F(">> STEALTH MODE: ON (SAVED)"));
    } else {
        Serial.println(F(">> STEALTH MODE: OFF (SAVED)"));
    }
}

void SystemController::cycleSystemMode() {
    _data.isWarmup = false;
    if (_data.currentMode == MODE_ACTIVE) {
        _data.currentMode = MODE_PASSIVE;
    } else {
        _data.currentMode = MODE_ACTIVE;
    }

    saveSettings();

    _sensorAwake = true;
    _pmSensor.wakeup();
    _stateTimer = millis();
    _isDataFresh = false;
    Serial.print(F(">> MODE CHANGED & SAVED: "));
    Serial.println(_data.currentMode == MODE_ACTIVE ? "ACTIVE" : "PASSIVE");
}

void SystemController::handlePMSensor() {
    unsigned long now = millis();
    unsigned long elapsed = now - _stateTimer;

    if (_data.currentMode == MODE_ACTIVE) {
        handleActiveMode();
    } else {
        handlePassiveMode(now, elapsed);
    }
}

void SystemController::handleActiveMode() {
    if (_pmSensor.readData(_data.pm1_0, _data.pm2_5, _data.pm10_0)) {
        _isDataFresh = true;
    }
}

void SystemController::handlePassiveMode(unsigned long now, unsigned long elapsed) {
    if (_sensorAwake) {
        if (_pmSensor.readData(_data.pm1_0, _data.pm2_5, _data.pm10_0)) {
            if (elapsed > STABILITY_THRESHOLD) _isDataFresh = true;
        }
        if (elapsed >= PM_WAKE_DURATION) {
            _pmSensor.sleep();
            _sensorAwake = false;
            _stateTimer = now;
            _isDataFresh = false;
        }
    } else {
        if (elapsed >= PM_SLEEP_DURATION) {
            _pmSensor.wakeup();
            _sensorAwake = true;
            _stateTimer = now;
        }
    }
}

void SystemController::updateUI() {
    unsigned long now = millis();
    _data.wifiStatus = _wifi.getWifiStatus();
    _data.isSleeping = !_sensorAwake;
    _data.showBlynkIcon = (now - _blynkFlashTimer < BLYNK_ICON_KEEP_ALIVE);

    unsigned long elapsed = now - _stateTimer;
    if (_data.currentMode == MODE_ACTIVE) {
        _data.countdown = 0;
    } else {
        unsigned long limit = _sensorAwake ? PM_WAKE_DURATION : PM_SLEEP_DURATION;
        _data.countdown = (limit - elapsed) / 1000;
    }
    _oled.update(_data);
}

void SystemController::handleBlynkTransmission() {
    unsigned long now = millis();
    if ((WiFi.status() == WL_CONNECTED) && _isDataFresh && (now - _lastBlynk > BLYNK_SEND_INTERVAL)) {
        _blynk.sendData(BLYNK_AUTH_TOKEN, _data.pm1_0, _data.pm2_5, _data.pm10_0, _data.temp, _data.hum);
        _lastBlynk = now;
        _blynkFlashTimer = now;
        _isDataFresh = false;
    }
}

void SystemController::loadSettings() {
    byte savedMode;
    EEPROM.get(ADDR_MODE, savedMode);
    if (savedMode == 0 || savedMode == 1) {
        _data.currentMode = (SystemMode)savedMode;
    } else {
        _data.currentMode = (SystemMode)DEFAULT_MODE_SETTING;
    }

    byte savedMuted;
    EEPROM.get(ADDR_MUTED, savedMuted);
    if (savedMuted == 255) {
        _isMuted = false;
    } else {
        _isMuted = (bool)savedMuted;
    }
}

void SystemController::saveSettings() {
    EEPROM.put(ADDR_MODE, (byte)_data.currentMode);
    EEPROM.put(ADDR_MUTED, (byte)_isMuted);
    EEPROM.commit();
}
