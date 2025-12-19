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
const char* FIRMWARE_VERSION = "V1.3.0 - Pro UI";
const unsigned long INITIAL_WARMUP_DURATION = 300000; // 5 mins
const unsigned long PM_WAKE_DURATION = 35000;         // 35s wake (extra 5s for stability)
const unsigned long PM_SLEEP_DURATION = 120000;       // 2 mins sleep
const unsigned long BLYNK_SEND_INTERVAL = 60000;      // 1 min throttle
const unsigned long STABILITY_THRESHOLD = 30000;      // 30s stable wait before "fresh"
const unsigned long BLYNK_ICON_KEEP_ALIVE = 1000;     // Show 'B' for 1 seconds

// --- Global Instances ---
WiFiHandler wifiHandler;
ResetHandler resetHandler(wifiHandler);
PMSensor pmSensor(PM_SENSOR_RX_PIN, PM_SENSOR_SET_PIN); 
BlynkHandler blynkHandler;
DHTSensor dhtSensor;
OLEDDisplay oledDisplay;
RGBLEDHandler rgbLEDHandler(WS2812_LED_PIN); // Initialize for WS2812 on D3

// --- Global State ---
float pm1_0_val, pm2_5_val, pm10_0_val, temp_val, hum_val;
unsigned long lastBlynkSend = 0;
unsigned long stateTimer = 0;
unsigned long bootTime = 0;
unsigned long blynkIconTimer = 0;

bool sensorIsAwake = true;
bool isInitialWarmup = true;
bool isDataFresh = false; 

// ----------------------------------------------------------------------
// SETUP
// ----------------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n--- System Booting ---");
    Serial.printf("Firmware: %s\n", FIRMWARE_VERSION);
    
    // 1. Initialize Displays & Indicators
    oledDisplay.setup();
    rgbLEDHandler.setup();
    rgbLEDHandler.startupSequence(); // Will cycle Red-Green-Blue on WS2812
    
    // 2. Hardware Connectivity
    resetHandler.checkPowerCycles();
    wifiHandler.startConnect();
    
    // 3. Sensors
    pmSensor.begin(9600);
    dhtSensor.setup();

    bootTime = millis();
    stateTimer = millis();
    Serial.println(">> MODE: Initial 5-minute warmup started.");
}

// ----------------------------------------------------------------------
// MAIN LOOP
// ----------------------------------------------------------------------

void loop() {
    wifiHandler.handleConnect();
    
    handlePMSensor();
    updateSecondarySensors();
    updateDisplays();
    handleCloudUpdates();

    delay(1000); 
}

// ----------------------------------------------------------------------
// 🛠️ SYSTEM METHODS
// ----------------------------------------------------------------------

void handlePMSensor() {
    unsigned long currentMillis = millis();
    unsigned long timeInState = currentMillis - stateTimer;

    if (isInitialWarmup) {
        if (pmSensor.readData(pm1_0_val, pm2_5_val, pm10_0_val)) {
            Serial.printf("[WARMUP] PM2.5: %.0f\n", pm2_5_val);
            rgbLEDHandler.updateLED(pm2_5_val, true);
            
            // Only allow data to be considered "fresh" after 30s of initial boot
            if (currentMillis - bootTime > STABILITY_THRESHOLD) {
                isDataFresh = true;
            }
        }

        if (currentMillis - bootTime >= INITIAL_WARMUP_DURATION) {
            isInitialWarmup = false;
            stateTimer = currentMillis;
            Serial.println(">> MODE: Warmup complete.");
        }
    } 
    else if (sensorIsAwake) {
        if (pmSensor.readData(pm1_0_val, pm2_5_val, pm10_0_val)) {
            Serial.printf("[ACTIVE] PM2.5: %.0f\n", pm2_5_val);
            rgbLEDHandler.updateLED(pm2_5_val, true);

            // Only mark data as fresh if fan has been running > 30s
            if (timeInState > STABILITY_THRESHOLD) {
                isDataFresh = true; 
            }
        }

        if (timeInState >= PM_WAKE_DURATION) {
            pmSensor.sleep();
            sensorIsAwake = false;
            stateTimer = currentMillis;
            Serial.println(">> SENSOR: Entering sleep.");
        }
    } 
    else {
        // While sleeping, we don't set isDataFresh to false.
        // This lets Blynk send the last stable reading during the sleep gap.
        if (timeInState >= PM_SLEEP_DURATION) {
            Serial.println(">> SENSOR: Waking up.");
            pmSensor.wakeup();
            pmSensor.clearBuffer();
            sensorIsAwake = true;
            stateTimer = currentMillis;
            
            // Reset freshness ONLY at wakeup so we don't send unstable packets
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

    // Calculate seconds remaining for current state
    if (isInitialWarmup) {
        countdown = (INITIAL_WARMUP_DURATION - (currentMillis - bootTime)) / 1000;
    } else if (sensorIsAwake) {
        countdown = (PM_WAKE_DURATION - timeInState) / 1000;
    } else {
        countdown = (PM_SLEEP_DURATION - timeInState) / 1000;
    }
    if (countdown < 0) countdown = 0;

    // Check if we should show the Blynk 'B' icon
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
        Serial.println(">> BLYNK: Sending stable measurement...");
        blynkHandler.sendData(BLYNK_AUTH_TOKEN, pm1_0_val, pm2_5_val, pm10_0_val, temp_val, hum_val);
        
        lastBlynkSend = now;
        isDataFresh = false; 
        blynkIconTimer = now; // Trigger the 'B' icon on OLED
    }
}
