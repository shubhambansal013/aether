#include <Arduino.h>
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

// --- State Management ---
unsigned long lastBlynk = 0;
unsigned long stateTimer = 0;
unsigned long bootTime = 0;
unsigned long blynkFlashTimer = 0;

bool sensorAwake = true;
bool isDataFresh = false;
bool isMuted = false; // Stealth mode flag

// --- Function Prototypes ---
void handlePMSensor();
void updateUI();
void handleBlynkTransmission();
void cycleSystemMode();
void toggleStealthMode();

void setup() {
    Serial.begin(115200);
    
    button.setup();
    oled.setup();
    led.setup();
    led.startupSequence();
    
    pmSensor.begin(9600);
    dhtSensor.setup();
    wifi.startConnect();

    data.currentMode = (SystemMode)DEFAULT_MODE_SETTING;
    data.isWarmup = false; 
    
    sensorAwake = true;
    pmSensor.wakeup();
    
    bootTime = millis();
    stateTimer = millis();
    
    Serial.println(F("--- System Initialized ---"));
}

void loop() {
    wifi.handleConnect();
    
    // Short Press: Change Active/Passive Mode
    if (button.isPressed()) {
        cycleSystemMode();
    }

    // Long Press: Toggle Display and LED
    if (button.isLongPressed()) {
        toggleStealthMode();
    }
    
    handlePMSensor();
    
    data.temp = dhtSensor.getTemperature();
    data.hum = dhtSensor.getHumidity();
    
    // Visual Updates
    if (!isMuted) {
        updateUI();
        led.updateLED(data.pm2_5);
    } else {
        // Optional: ensure hardware stays off even if background tasks run
        oled.clear(); 
        led.updateLED(0); // Assuming 0 turns off/darkens the LED
    }

    handleBlynkTransmission();
    delay(20); 
}

void toggleStealthMode() {
    isMuted = !isMuted;
    if (isMuted) {
        oled.clear(); 
        Serial.println(F(">> STEALTH MODE: ON"));
    } else {
        Serial.println(F(">> STEALTH MODE: OFF"));
    }
}

void cycleSystemMode() {
    data.isWarmup = false;
    if (data.currentMode == MODE_ACTIVE) {
        data.currentMode = MODE_PASSIVE;
        sensorAwake = true;
        pmSensor.wakeup();
        Serial.println(F(">> MODE: PASSIVE"));
    } else {
        data.currentMode = MODE_ACTIVE;
        sensorAwake = true;
        pmSensor.wakeup();
        Serial.println(F(">> MODE: ACTIVE"));
    }
    stateTimer = millis();
    isDataFresh = false;
}

void handlePMSensor() {
    unsigned long now = millis();
    unsigned long elapsed = now - stateTimer;

    if (data.currentMode == MODE_ACTIVE) {
        if (pmSensor.readData(data.pm1_0, data.pm2_5, data.pm10_0)) isDataFresh = true;
        return;
    }

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
    if ((WiFi.status() == WL_CONNECTED) && isDataFresh && (now - lastBlynk > BLYNK_SEND_INTERVAL)) {
        blynk.sendData(BLYNK_AUTH_TOKEN, data.pm1_0, data.pm2_5, data.pm10_0, data.temp, data.hum);
        lastBlynk = now;
        blynkFlashTimer = now; 
        isDataFresh = false; 
    }
}
