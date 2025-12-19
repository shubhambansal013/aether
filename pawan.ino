#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include "WiFiHandler.h"
#include "ResetHandler.h"
#include "PMSensor.h"
#include "BlynkHandler.h"
#include "blynk_config.h"
#include "pins.h"
#include "OTAHandler.h"
#include "DHTSensor.h"
#include "OLEDDisplay.h"
#include "RGBLEDHandler.h"

// --- Constants ---
const char* FIRMWARE_VERSION = "V1.2.2 - PM sensor sleep/wakeup";
const unsigned long INITIAL_WARMUP_DURATION = 300000; 
const unsigned long PM_WAKE_DURATION = 30000;         
const unsigned long PM_SLEEP_DURATION = 120000;       
const unsigned long BLYNK_SEND_INTERVAL = 60000; 
const unsigned long STABILITY_THRESHOLD = 15000; // Wait 15s after wake before sending to Blynk

// --- Instances ---
WiFiHandler wifiHandler;
ResetHandler resetHandler(wifiHandler);
PMSensor pmSensor(PM_SENSOR_RX_PIN, PM_SENSOR_SET_PIN); 
BlynkHandler blynkHandler;
OTAHandler otaHandler(FIRMWARE_VERSION, GITHUB_REPO_USER, GITHUB_REPO_NAME, FIRMWARE_BIN_NAME);
DHTSensor dhtSensor;
OLEDDisplay oledDisplay;
RGBLEDHandler rgbLEDHandler(RGB_LED_RED_PIN, RGB_LED_GREEN_PIN, RGB_LED_BLUE_PIN);

// --- Global State ---
float pm1_0_val, pm2_5_val, pm10_0_val, temp_val, hum_val;
unsigned long lastBlynkSend = 0;
unsigned long stateTimer = 0;
unsigned long bootTime = 0;

bool sensorIsAwake = true;
bool isInitialWarmup = true;
bool isDataFresh = false; // Flag to track if we have a new, stable reading to send

void setup() {
    Serial.begin(115200);
    oledDisplay.setup();
    oledDisplay.printMessage("System", "Booting...");
    
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

// ----------------------------------------------------------------------
// 🛠️ REFACTORED METHODS
// ----------------------------------------------------------------------

void handlePMSensor() {
    unsigned long currentMillis = millis();
    unsigned long timeInState = currentMillis - stateTimer;

    if (isInitialWarmup) {
        bool success = pmSensor.readData(pm1_0_val, pm2_5_val, pm10_0_val);
        if (success) {
            rgbLEDHandler.updateLED(pm2_5_val, true);
            // In warmup, data is "stable" after the first 15s of boot
            if (currentMillis - bootTime > STABILITY_THRESHOLD) isDataFresh = true;
        }

        if (currentMillis - bootTime >= INITIAL_WARMUP_DURATION) {
            isInitialWarmup = false;
            stateTimer = currentMillis;
            Serial.println("Warmup complete.");
        }
    } 
    else if (sensorIsAwake) {
        bool success = pmSensor.readData(pm1_0_val, pm2_5_val, pm10_0_val);
        if (success) {
            rgbLEDHandler.updateLED(pm2_5_val, true);
            // Only mark data as fresh/stable if sensor has been awake for > 15s
            if (timeInState > STABILITY_THRESHOLD) {
                isDataFresh = true;
            }
        }

        if (timeInState >= PM_WAKE_DURATION) {
            pmSensor.sleep();
            sensorIsAwake = false;
            stateTimer = currentMillis;
        }
    } 
    else {
        // Sensor is sleeping
        if (timeInState >= PM_SLEEP_DURATION) {
            pmSensor.wakeup();
            pmSensor.clearBuffer();
            sensorIsAwake = true;
            stateTimer = currentMillis;
            isDataFresh = false; // Reset freshness for the new wake cycle
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
    unsigned long currentMillis = millis();
    bool currentlyConnected = (WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_STA);

    // Criteria: 1. Connected, 2. Timer > 1min, 3. Data is fresh and stable
    if (currentlyConnected && (currentMillis - lastBlynkSend > BLYNK_SEND_INTERVAL)) {
        if (isDataFresh) {
            blynkHandler.sendData(BLYNK_AUTH_TOKEN, pm1_0_val, pm2_5_val, pm10_0_val, temp_val, hum_val);
            lastBlynkSend = currentMillis;
            isDataFresh = false; // Data has been sent, wait for next stable window
            Serial.println("Blynk Update: Stable data sent.");
        }
    }
}
