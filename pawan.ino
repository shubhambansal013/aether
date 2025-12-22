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
    
    // Initialize Hardware
    button.setup();
    oled.setup();
    led.setup();
    led.startupSequence();
    
    pmSensor.begin(9600);
    dhtSensor.setup();
    wifi.startConnect();
    
    bootTime = millis();
    stateTimer = millis();
    
    Serial.println(F("--- System Initialized (3-Mode Edition) ---"));
}

void loop() {
    // 1. Maintain Background Services
    wifi.handleConnect();
    
    // 2. Process Manual Mode Toggle (Button Class Handles Debounce/Edges)
    if (button.isPressed()) {
        cycleSystemMode();
    }
    
    // 3. Process Sensor Lifecycles
    handlePMSensor();
    
    // 4. Update Secondary Sensors (Encapsulated)
    data.temp = dhtSensor.getTemperature();
    data.hum = dhtSensor.getHumidity();
    
    // 5. Refresh Visuals & Cloud
    updateUI();
    handleBlynkTransmission();
    
    // 6. LED Logic (Persistence during sleep)
    led.updateLED(data.pm2_5);

    // Small delay to prevent CPU hogging, Button class makes this responsive
    delay(20); 
}

/**
 * @brief Logic for cycling through 3 user modes.
 * Transitions: AUTO -> MANUAL ACTIVE -> MANUAL PASSIVE -> AUTO
 */
void cycleSystemMode() {
    // Any button press exits the initial warmup immediately
    data.isWarmup = false;

    if (data.currentMode == MODE_AUTO) {
        data.currentMode = MODE_ACTIVE;
        sensorAwake = true;
        pmSensor.wakeup();
        Serial.println(F(">> MODE SWITCH: MANUAL ACTIVE (Always On)"));
    } 
    else if (data.currentMode == MODE_ACTIVE) {
        data.currentMode = MODE_PASSIVE;
        sensorAwake = false; // Start passive mode with a sleep to save fan life
        pmSensor.sleep();
        Serial.println(F(">> MODE SWITCH: MANUAL PASSIVE (Eco Duty Cycle)"));
    } 
    else {
        data.currentMode = MODE_AUTO;
        sensorAwake = true;
        pmSensor.wakeup();
        Serial.println(F(">> MODE SWITCH: AUTO (Duty Cycle)"));
    }
    
    // Sync timers and data flags
    stateTimer = millis();
    isDataFresh = false;
}

/**
 * @brief State machine for the PM sensor.
 * Now respects the Manual Mode overrides.
 */
void handlePMSensor() {
    unsigned long now = millis();
    unsigned long elapsed = now - stateTimer;

    // A. MANUAL ACTIVE: Continuous operation, no sleep timers.
    if (data.currentMode == MODE_ACTIVE) {
        if (pmSensor.readData(data.pm1_0, data.pm2_5, data.pm10_0)) {
            isDataFresh = true; // Data is considered fresh as long as fan is on
        }
        return; 
    }

    // B. AUTO or PASSIVE logic
    if (sensorAwake) {
        if (pmSensor.readData(data.pm1_0, data.pm2_5, data.pm10_0)) {
            if (elapsed > STABILITY_THRESHOLD) isDataFresh = true;
        }

        // Determine when to sleep
        // PASSIVE mode sleeps after 35s. AUTO stays awake for standard WAKE_DURATION.
        unsigned long wakeLimit = (data.currentMode == MODE_PASSIVE) ? 35000 : PM_WAKE_DURATION;
        
        if (elapsed >= wakeLimit) {
            pmSensor.sleep();
            sensorAwake = false;
            stateTimer = now;
            isDataFresh = false;
        }
    } else {
        // Shared sleep duration
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

    // Flash Blynk 'B' icon only during successful transmission window
    data.showBlynkIcon = (now - blynkFlashTimer < BLYNK_ICON_KEEP_ALIVE);

    // Calculate Countdown based on current mode timer
    unsigned long elapsed = now - stateTimer;
    if (data.isWarmup) {
        data.countdown = (INITIAL_WARMUP_DURATION - (now - bootTime)) / 1000;
    } else if (data.currentMode == MODE_ACTIVE) {
        data.countdown = 0; // No countdown in manual active
    } else {
        unsigned long limit = sensorAwake ? 
                             ((data.currentMode == MODE_PASSIVE) ? 35 : (PM_WAKE_DURATION/1000)) : 
                             (PM_SLEEP_DURATION/1000);
        
        // Note: For simplicity, use raw seconds here if PM_WAKE_DURATION is in ms
        int totalLimit = sensorAwake ? 
                        ((data.currentMode == MODE_PASSIVE) ? 35 : (PM_WAKE_DURATION/1000)) : 
                        (PM_SLEEP_DURATION/1000);
        
        data.countdown = totalLimit - (elapsed / 1000);
    }
    
    if (data.countdown < 0) data.countdown = 0;
    oled.update(data);
}

void handleBlynkTransmission() {
    unsigned long now = millis();
    bool wifiConnected = (WiFi.status() == WL_CONNECTED);

    if (wifiConnected && isDataFresh && (now - lastBlynk > BLYNK_SEND_INTERVAL)) {
        blynk.sendData(BLYNK_AUTH_TOKEN, data.pm1_0, data.pm2_5, data.pm10_0, data.temp, data.hum);
        lastBlynk = now;
        blynkFlashTimer = now; // Start the 3s visible 'B' indicator
        isDataFresh = false; 
    }
}
