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
const char* FIRMWARE_VERSION = "V1.2.6 - Debug Enhanced";
const unsigned long INITIAL_WARMUP_DURATION = 300000; 
const unsigned long PM_WAKE_DURATION = 30000;         
const unsigned long PM_SLEEP_DURATION = 120000;       
const unsigned long BLYNK_SEND_INTERVAL = 60000;      
const unsigned long STABILITY_THRESHOLD = 15000;      

// --- Global State ---
WiFiHandler wifiHandler;
ResetHandler resetHandler(wifiHandler);
PMSensor pmSensor(PM_SENSOR_RX_PIN, PM_SENSOR_SET_PIN); 
BlynkHandler blynkHandler;
DHTSensor dhtSensor;
OLEDDisplay oledDisplay;
RGBLEDHandler rgbLEDHandler(RGB_LED_RED_PIN, RGB_LED_GREEN_PIN, RGB_LED_BLUE_PIN);

float pm1_0_val, pm2_5_val, pm10_0_val, temp_val, hum_val;
unsigned long lastBlynkSend = 0;
unsigned long stateTimer = 0;
unsigned long bootTime = 0;

bool sensorIsAwake = true;
bool isInitialWarmup = true;
bool isDataFresh = false; 

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n--- System Booting ---");
    Serial.printf("Firmware: %s\n", FIRMWARE_VERSION);
    
    oledDisplay.setup();
    rgbLEDHandler.setup();
    rgbLEDHandler.startupSequence();
    
    resetHandler.checkPowerCycles();
    wifiHandler.startConnect();
    
    pmSensor.begin(9600);
    dhtSensor.setup();

    bootTime = millis();
    stateTimer = millis();
    Serial.println("Initial 5-minute warmup started...");
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
            Serial.printf("[WARMUP] PM1.0: %.0f | PM2.5: %.0f | PM10: %.0f\n", pm1_0_val, pm2_5_val, pm10_0_val);
            rgbLEDHandler.updateLED(pm2_5_val, true);
            if (currentMillis - bootTime > STABILITY_THRESHOLD) isDataFresh = true;
        }
        if (currentMillis - bootTime >= INITIAL_WARMUP_DURATION) {
            isInitialWarmup = false;
            stateTimer = currentMillis;
            Serial.println(">> WARMUP COMPLETE. Switching to power-save cycles.");
        }
    } 
    else if (sensorIsAwake) {
        if (pmSensor.readData(pm1_0_val, pm2_5_val, pm10_0_val)) {
            Serial.printf("[ACTIVE] PM1.0: %.0f | PM2.5: %.0f | PM10: %.0f\n", pm1_0_val, pm2_5_val, pm10_0_val);
            rgbLEDHandler.updateLED(pm2_5_val, true);
            if (timeInState > STABILITY_THRESHOLD) isDataFresh = true;
        }
        if (timeInState >= PM_WAKE_DURATION) {
            pmSensor.sleep();
            sensorIsAwake = false;
            stateTimer = currentMillis;
            Serial.println(">> SENSOR SLEEPING: Hardware standby initiated.");
        }
    } 
    else {
        // Logging sleep progress every 30s
        if (timeInState % 30000 < 1000) { 
            Serial.printf(">> SENSOR ASLEEP: %lus remaining...\n", (PM_SLEEP_DURATION - timeInState) / 1000);
        }

        if (timeInState >= PM_SLEEP_DURATION) {
            Serial.println(">> SENSOR WAKING: Restarting fan and laser...");
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
    String status = wifiHandler.getWifiStatus();
    if (isInitialWarmup) status += " (Warmup)";
    else if (!sensorIsAwake) status += " (Zzz)"; 
    oledDisplay.displaySensorDataAndWifiStatus(status, pm1_0_val, pm2_5_val, pm10_0_val, hum_val, temp_val);
}

void handleCloudUpdates() {
    unsigned long now = millis();
    bool connected = (WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_STA);

    if (connected && (now - lastBlynkSend > BLYNK_SEND_INTERVAL)) {
        if (isDataFresh) {
            Serial.println(">> BLYNK: Sending stable PM and DHT data...");
            blynkHandler.sendData(BLYNK_AUTH_TOKEN, pm1_0_val, pm2_5_val, pm10_0_val, temp_val, hum_val);
            lastBlynkSend = now;
            isDataFresh = false; 
        } else {
            Serial.println(">> BLYNK: Waiting for data stability...");
        }
    }
}
