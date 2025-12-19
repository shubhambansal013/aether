#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include "WiFiHandler.h"
#include "ResetHandler.h"
#include "PMSensor.h"
#include "BlynkHandler.h"
#include "blynk_config.h"
#include "pins.h"
#include "DHTSensor.h"
#include "OLEDDisplay.h"
#include "RGBLEDHandler.h"

// --- Constants & Firmware Info ---
const char* FIRMWARE_VERSION = "V1.3.1 - PWM LED Restore";
const unsigned long INITIAL_WARMUP_DURATION = 300000; // 5 mins
const unsigned long PM_WAKE_DURATION = 35000;         // 35s wake
const unsigned long PM_SLEEP_DURATION = 120000;       // 2 mins sleep
const unsigned long BLYNK_SEND_INTERVAL = 60000;      // 1 min throttle
const unsigned long STABILITY_THRESHOLD = 30000;      // 30s stable wait
const unsigned long BLYNK_ICON_KEEP_ALIVE = 3000;     // 3 seconds

// --- Global Instances ---
WiFiHandler wifiHandler;
ResetHandler resetHandler(wifiHandler);
PMSensor pmSensor(PM_SENSOR_RX_PIN, PM_SENSOR_SET_PIN); 
BlynkHandler blynkHandler;
DHTSensor dhtSensor;
OLEDDisplay oledDisplay;
RGBLEDHandler rgbLEDHandler(RGB_LED_RED_PIN, RGB_LED_GREEN_PIN, RGB_LED_BLUE_PIN);

// --- Global State ---
float pm1_0_val, pm2_5_val, pm10_0_val, temp_val, hum_val;
unsigned long lastBlynkSend = 0;
unsigned long stateTimer = 0;
unsigned long bootTime = 0;
unsigned long blynkIconTimer = 0;

bool sensorIsAwake = true;
bool isInitialWarmup = true;
bool isDataFresh = false; 

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n--- System Booting ---");
    
    oledDisplay.setup();
    rgbLEDHandler.setup();
    rgbLEDHandler.startupSequence(); 
    
    resetHandler.checkPowerCycles();
    wifiHandler.startConnect();
    
    pmSensor.begin(9600);
    dhtSensor.setup();

    bootTime = millis();
    stateTimer = millis();
}

void loop() {
    wifiHandler.handleConnect();
    
    handlePMSensor();
    updateSecondarySensors();
    updateDisplays();
    handleCloudUpdates();

    delay(1000); 
}

void handlePMSensor() {
    unsigned long currentMillis = millis();
    unsigned long timeInState = currentMillis - stateTimer;

    if (isInitialWarmup) {
        if (pmSensor.readData(pm1_0_val, pm2_5_val, pm10_0_val)) {
            Serial.printf("[WARMUP] PM2.5: %.0f\n", pm2_5_val);
            rgbLEDHandler.updateLED(pm2_5_val, true);
            if (currentMillis - bootTime > STABILITY_THRESHOLD) isDataFresh = true;
        }
        if (currentMillis - bootTime >= INITIAL_WARMUP_DURATION) {
            isInitialWarmup = false;
            stateTimer = currentMillis;
            Serial.println(">> Warmup complete.");
        }
    } 
    else if (sensorIsAwake) {
        if (pmSensor.readData(pm1_0_val, pm2_5_val, pm10_0_val)) {
            Serial.printf("[ACTIVE] PM2.5: %.0f\n", pm2_5_val);
            rgbLEDHandler.updateLED(pm2_5_val, true);
            if (timeInState > STABILITY_THRESHOLD) isDataFresh = true;
        }
        if (timeInState >= PM_WAKE_DURATION) {
            pmSensor.sleep();
            sensorIsAwake = false;
            stateTimer = currentMillis;
            Serial.println(">> Sensor Sleeping.");
        }
    } 
    else {
        if (timeInState >= PM_SLEEP_DURATION) {
            Serial.println(">> Sensor Waking.");
            pmSensor.wakeup();
            pmSensor.clearBuffer();
            sensorIsAwake = true;
            stateTimer = currentMillis;
            isDataFresh = false; 
        }
    }
}

void updateSecondarySensors() {
    hum_val = dhtSensor.readHumidity();
    temp_val = dhtSensor.readTemperature();
}

void updateDisplays() {
    unsigned long currentMillis = millis();
    unsigned long timeInState = currentMillis - stateTimer;
    int countdown = 0;

    if (isInitialWarmup) {
        countdown = (INITIAL_WARMUP_DURATION - (currentMillis - bootTime)) / 1000;
    } else if (sensorIsAwake) {
        countdown = (PM_WAKE_DURATION - timeInState) / 1000;
    } else {
        countdown = (PM_SLEEP_DURATION - timeInState) / 1000;
    }
    if (countdown < 0) countdown = 0;

    bool showBlynk = (currentMillis - blynkIconTimer < BLYNK_ICON_KEEP_ALIVE);

    oledDisplay.update(
        wifiHandler.getWifiStatus(), 
        pm1_0_val, pm2_5_val, pm10_0_val, 
        temp_val, hum_val, 
        isInitialWarmup, 
        !sensorIsAwake,
        countdown,
        showBlynk
    );
}

void handleCloudUpdates() {
    unsigned long now = millis();
    bool connected = (WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_STA);

    if (connected && (now - lastBlynkSend > BLYNK_SEND_INTERVAL) && isDataFresh) {
        blynkHandler.sendData(BLYNK_AUTH_TOKEN, pm1_0_val, pm2_5_val, pm10_0_val, temp_val, hum_val);
        lastBlynkSend = now;
        isDataFresh = false; 
        blynkIconTimer = now; 
    }
}
