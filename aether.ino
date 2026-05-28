#include <Arduino.h>
#include <EEPROM.h>
#include "Config.h"
#include "SystemData.h"
#include "WiFiHandler.h"
#include "PMSensor.h"
#include "DHTSensor.h"
#include "OLEDDisplay.h"
#include "RGBLEDHandler.h"
#include "BlynkHandler.h"
#include "ButtonHandler.h"
#include "blynk_config.h"

// --- Shield for ESP8266 IRAM Errors ---
#ifndef IRAM_ATTR
  #define IRAM_ATTR __attribute__((section(".text")))
#endif

// --- Global Instances ---
SystemData data;
PMSensor pmSensor(PM_SENSOR_RX_PIN, PM_SENSOR_SET_PIN);
DHTSensor dhtSensor(DHT_PIN);
OLEDDisplay oled(OLED_SDA_PIN, OLED_SCL_PIN);
RGBLEDHandler led(WS2812_PIN); 
WiFiHandler wifi;
BlynkHandler blynk;
ButtonHandler button(BUTTON_PIN);
const int ADDR_MODE  = 10;
const int ADDR_MUTED = 14;

// --- Timers & Internal State ---
unsigned long lastBlynk = 0;
unsigned long stateTimer = 0;
unsigned long bootTime = 0;
unsigned long blynkFlashTimer = 0;

bool sensorAwake = true;
bool isDataFresh = false;

// --- Function Prototypes ---
void handlePMSensor();
void updateUI();
void handleBlynkTransmission();
void cycleSystemMode();
void saveSettings();
void loadSettings();

void setup() {
    Serial.begin(115200);
    EEPROM.begin(512);
    
    // 1. Initialize Hardware
    button.setup();
    oled.setup();
    led.setup();

    loadSettings();

    if (!data.isMuted) {
        led.startupSequence();
    }
    
    pmSensor.begin(9600);
    dhtSensor.setup();
    wifi.startConnect();

    // 2. Initial Mode (Simplified to 2 Phases)
    data.isWarmup = false; 
    
    // Both modes start with sensor awake
    sensorAwake = true;
    pmSensor.wakeup();
    
    bootTime = millis();
    stateTimer = millis();
    
    Serial.print(F("--- System Initialized. Mode: "));
    Serial.println(data.currentMode == MODE_ACTIVE ? "ACTIVE" : "PASSIVE");
}

void loop() {
    wifi.handleConnect();
    
    if (button.isPressed()) {
        cycleSystemMode();
    }
    
    if (button.isLongPressed()) {
        data.isMuted = !data.isMuted;
        if (data.isMuted) {
            oled.clear();
            led.turnOff();
            Serial.println(F(">> STEALTH MODE: ON"));
        } else {
            Serial.println(F(">> STEALTH MODE: OFF"));
        }
        saveSettings();
    }

    handlePMSensor();
    
    data.temp = dhtSensor.getTemperature();
    data.hum = dhtSensor.getHumidity();
    
    if (!data.isMuted) {
        updateUI();
        led.updateLED(data.pm2_5);
    }
    
    handleBlynkTransmission(); // Blynk logic remains untouched
    delay(20); 
}

/**
 * @brief Simple 2-mode cycle: ACTIVE <-> PASSIVE
 * Passive starts with Wake (Fan On).
 */
void cycleSystemMode() {
    data.isWarmup = false;

    if (data.currentMode == MODE_ACTIVE) {
        data.currentMode = MODE_PASSIVE;
        sensorAwake = true; // Start Passive with fan ON
        pmSensor.wakeup();
        Serial.println(F(">> MODE: PASSIVE (Starting Cycle)"));
    } 
    else {
        data.currentMode = MODE_ACTIVE;
        sensorAwake = true;
        pmSensor.wakeup();
        Serial.println(F(">> MODE: ACTIVE (Always On)"));
    }
    
    saveSettings();
    stateTimer = millis();
    isDataFresh = false;
}

void handlePMSensor() {
    unsigned long now = millis();
    unsigned long elapsed = now - stateTimer;

    // 1. MODE: ACTIVE - Stay awake forever
    if (data.currentMode == MODE_ACTIVE) {
        if (pmSensor.readData(data.pm1_0, data.pm2_5, data.pm10_0)) isDataFresh = true;
        return;
    }

    // 2. MODE: PASSIVE - (Wake -> Sleep cycle)
    if (sensorAwake) {
        if (pmSensor.readData(data.pm1_0, data.pm2_5, data.pm10_0)) {
            if (elapsed > STABILITY_THRESHOLD) isDataFresh = true;
        }
        if (elapsed >= PM_WAKE_DURATION) {
            pmSensor.sleep();
            sensorAwake = false;
            stateTimer = now;
            isDataFresh = false;
        }
    } else {
        if (elapsed >= PM_SLEEP_DURATION) {
            pmSensor.wakeup();
            sensorAwake = true;
            stateTimer = now;
        }
    }
}

void updateUI() {
    unsigned long now = millis();
    data.wifiStatus = wifi.getWifiStatus();
    data.isSleeping = !sensorAwake;

    data.showBlynkIcon = (now - blynkFlashTimer < BLYNK_ICON_KEEP_ALIVE);

    unsigned long elapsed = now - stateTimer;
    if (data.currentMode == MODE_ACTIVE) {
        data.countdown = 0; 
    } else {
        unsigned long limit = sensorAwake ? PM_WAKE_DURATION : PM_SLEEP_DURATION;
        data.countdown = (limit - elapsed) / 1000;
    }
    
    if (data.countdown < 0) data.countdown = 0;
    oled.update(data);
}

void handleBlynkTransmission() {
    unsigned long now = millis();
    // Logic exactly as before
    if ((WiFi.status() == WL_CONNECTED) && isDataFresh && (now - lastBlynk > BLYNK_SEND_INTERVAL)) {
        blynk.sendData(BLYNK_AUTH_TOKEN, data.pm1_0, data.pm2_5, data.pm10_0, data.temp, data.hum);
        lastBlynk = now;
        blynkFlashTimer = now; 
        isDataFresh = false; 
    }
}

void saveSettings() {
    EEPROM.put(ADDR_MODE, (int)data.currentMode);
    EEPROM.put(ADDR_MUTED, (int)data.isMuted);
    EEPROM.commit();
    Serial.println(F(">> Settings Saved to EEPROM."));
}

void loadSettings() {
    int storedMode;
    int storedMuted;
    EEPROM.get(ADDR_MODE, storedMode);
    EEPROM.get(ADDR_MUTED, storedMuted);

    // Validate stored mode (0: ACTIVE, 1: PASSIVE)
    if (storedMode >= 0 && storedMode <= 1) {
        data.currentMode = (SystemMode)storedMode;
        Serial.print(F(">> Loaded Mode: "));
        Serial.println(storedMode == MODE_ACTIVE ? "ACTIVE" : "PASSIVE");
    } else {
        data.currentMode = (SystemMode)DEFAULT_MODE_SETTING;
        Serial.println(F(">> EEPROM Mode Empty/Corrupt. Using Default."));
    }

    // Validate stored muted state
    if (storedMuted == 0 || storedMuted == 1) {
        data.isMuted = (bool)storedMuted;
        Serial.print(F(">> Loaded Stealth: "));
        Serial.println(data.isMuted ? "ON" : "OFF");
    } else {
        data.isMuted = false;
        Serial.println(F(">> EEPROM Muted Empty/Corrupt. Defaulting OFF."));
    }
}
