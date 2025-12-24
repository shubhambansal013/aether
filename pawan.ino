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

void setup() {
    Serial.begin(115200);
    
    // 1. Initialize Hardware Classes
    button.setup();
    oled.setup();
    led.setup();
    led.startupSequence();
    
    pmSensor.begin(9600);
    dhtSensor.setup();
    wifi.startConnect();

    // 2. Set Initial Mode from Config
    data.currentMode = (SystemMode)DEFAULT_MODE_SETTING;
    data.isWarmup = (data.currentMode == MODE_AUTO);
    
    // 3. Set Initial Sensor State
    // Active and Auto start with fan ON. Passive starts ON for its first cycle.
    sensorAwake = true;
    pmSensor.wakeup();
    
    bootTime = millis();
    stateTimer = millis();
    
    Serial.print(F("--- System Initialized. Mode: "));
    Serial.println(data.currentMode == MODE_AUTO ? "AUTO" : (data.currentMode == MODE_ACTIVE ? "ACTIVE" : "PASSIVE"));
}

void loop() {
    // A. Background WiFi/System maintenance
    wifi.handleConnect();
    
    // B. Process Manual Mode Toggle
    // Integrator-based class handles the 10k resistor noise filtering
    if (button.isPressed()) {
        cycleSystemMode();
    }
    
    // C. Process PM Sensor lifecycle based on Mode
    handlePMSensor();
    
    // D. Update Secondary Sensors (Encapsulated error handling)
    data.temp = dhtSensor.getTemperature();
    data.hum = dhtSensor.getHumidity();
    
    // E. Refresh Displays & Cloud
    updateUI();
    handleBlynkTransmission();
    
    // F. LED stays updated even during sensor sleep
    led.updateLED(data.pm2_5);

    delay(20); 
}

/**
 * @brief Transitions: AUTO -> PASSIVE -> ACTIVE -> AUTO
 */
void cycleSystemMode() {
    if (data.currentMode == MODE_AUTO) {
        data.currentMode = MODE_PASSIVE;
        data.isWarmup = false;
        sensorAwake = false; // Start passive mode with a sleep
        pmSensor.sleep();
        Serial.println(F(">> MODE: PASSIVE (Cycle Only)"));
    } 
    else if (data.currentMode == MODE_PASSIVE) {
        data.currentMode = MODE_ACTIVE;
        data.isWarmup = false;
        sensorAwake = true;
        pmSensor.wakeup();
        Serial.println(F(">> MODE: ACTIVE (Always On)"));
    } 
    else {
        data.currentMode = MODE_AUTO;
        data.isWarmup = true; // Auto starts with warmup
        bootTime = millis();
        sensorAwake = true;
        pmSensor.wakeup();
        Serial.println(F(">> MODE: AUTO (Warmup then Cycle)"));
    }
    stateTimer = millis();
    isDataFresh = false;
}

void handlePMSensor() {
    unsigned long now = millis();
    unsigned long elapsed = now - stateTimer;

    // 1. MODE: ACTIVE - No timers, just stay awake
    if (data.currentMode == MODE_ACTIVE) {
        if (pmSensor.readData(data.pm1_0, data.pm2_5, data.pm10_0)) isDataFresh = true;
        return;
    }

    // 2. MODE: AUTO (Initial Warmup Phase)
    if (data.currentMode == MODE_AUTO && data.isWarmup) {
        if (pmSensor.readData(data.pm1_0, data.pm2_5, data.pm10_0)) {
            if (now - bootTime > STABILITY_THRESHOLD) isDataFresh = true;
        }
        if (now - bootTime >= INITIAL_WARMUP_DURATION) {
            data.isWarmup = false; // Move to cycle phase
            stateTimer = now;
        }
        return;
    }

    // 3. CYCLE LOGIC (Used by PASSIVE and Post-Warmup AUTO)
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

    // Visible Blynk 'B' indicator duration
    data.showBlynkIcon = (now - blynkFlashTimer < BLYNK_ICON_KEEP_ALIVE);

    // Countdown logic
    unsigned long elapsed = now - stateTimer;
    if (data.isWarmup) {
        data.countdown = (INITIAL_WARMUP_DURATION - (now - bootTime)) / 1000;
    } else if (data.currentMode == MODE_ACTIVE) {
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
