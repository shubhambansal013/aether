#include <Arduino.h>
#include "Config.h"
#include "SystemData.h"
#include "WiFiHandler.h"
#include "PMSensor.h"
#include "DHTSensor.h"
#include "OLEDDisplay.h"
#include "RGBLEDHandler.h"
#include "BlynkHandler.h"
#include "blynk_config.h"

// --- Shield for ESP8266 IRAM Errors ---
#ifndef IRAM_ATTR
  #define IRAM_ATTR __attribute__((section(".text")))
#endif

// --- Instances ---
SystemData data;
PMSensor pmSensor(PM_SENSOR_RX_PIN, PM_SENSOR_SET_PIN);
DHTSensor dhtSensor(DHT_PIN);
OLEDDisplay oled(OLED_SDA_PIN, OLED_SCL_PIN);
RGBLEDHandler led(WS2812_PIN); 
WiFiHandler wifi;
BlynkHandler blynk;

// --- Timers & State ---
unsigned long lastBlynk = 0;
unsigned long stateTimer = 0;
unsigned long bootTime = 0;
unsigned long blynkFlashTimer = 0;
unsigned long lastButtonPress = 0;

const unsigned long DEBOUNCE_DELAY = 300; 

bool sensorAwake = true;
bool isDataFresh = false;

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    oled.setup();
    led.setup();
    led.startupSequence();
    
    pmSensor.begin(9600);
    dhtSensor.setup();
    wifi.startConnect();
    
    bootTime = millis();
    stateTimer = millis();
    Serial.println(F("--- System Initialized ---"));
}

void loop() {
    wifi.handleConnect();
    handleButton();
    handlePMSensor();
    
    data.temp = dhtSensor.getTemperature();
    data.hum = dhtSensor.getHumidity();
    
    updateUI();
    handleBlynkTransmission();
    led.updateLED(data.pm2_5);

    delay(100); 
}

/**
 * @brief Handles manual toggle. Now allows bypassing warmup.
 */
void handleButton() {
    static bool lastPhysicalState = HIGH; // Stores the previous loop's reading
    bool currentPhysicalState = digitalRead(BUTTON_PIN);
    unsigned long now = millis();

    // 1. Check for the "Falling Edge" (Transition from HIGH to LOW)
    if (currentPhysicalState == LOW && lastPhysicalState == HIGH) {
        
        // 2. Apply Debounce Time Filter
        if (now - lastButtonPress > 300) { 
            lastButtonPress = now;

            // Handle the Mode Toggle
            if (data.isWarmup) data.isWarmup = false;

            if (sensorAwake) {
                Serial.println(F(">> Button Action: SLEEP"));
                pmSensor.sleep();
                sensorAwake = false;
            } else {
                Serial.println(F(">> Button Action: WAKEUP"));
                pmSensor.wakeup();
                sensorAwake = true;
            }

            stateTimer = now;
            isDataFresh = false; 
        }
    }
    
    // 3. Update the state for the next loop iteration
    lastPhysicalState = currentPhysicalState;
}
/**
 * @brief Manages sensor cycles.
 */
void handlePMSensor() {
    unsigned long now = millis();
    unsigned long elapsed = now - stateTimer;

    // Phase 1: Initial Warmup (Only runs if not bypassed by button)
    if (data.isWarmup) {
        if (pmSensor.readData(data.pm1_0, data.pm2_5, data.pm10_0)) {
            if (now - bootTime > STABILITY_THRESHOLD) isDataFresh = true;
        }
        if (now - bootTime >= INITIAL_WARMUP_DURATION) {
            data.isWarmup = false; 
            stateTimer = now;
            Serial.println(F(">> SENSOR: Warmup Complete."));
        }
    } 
    // Phase 2: Active Duty Cycle
    else if (sensorAwake) {
        if (pmSensor.readData(data.pm1_0, data.pm2_5, data.pm10_0)) {
            if (elapsed > STABILITY_THRESHOLD) isDataFresh = true;
        }
        if (elapsed >= PM_WAKE_DURATION) {
            pmSensor.sleep(); 
            sensorAwake = false; 
            stateTimer = now; 
            isDataFresh = false; 
        }
    } 
    // Phase 3: Passive Cycle
    else {
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
    
    // UI reflects the current mode immediately
    if (data.isWarmup) {
        data.countdown = (INITIAL_WARMUP_DURATION - (now - bootTime)) / 1000;
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
