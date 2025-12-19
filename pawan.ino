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
const char* FIRMWARE_VERSION = "V1.2.8 - Stability Lock";
const unsigned long INITIAL_WARMUP_DURATION = 300000; // 5 mins
const unsigned long PM_WAKE_DURATION = 35000;         // 35s wake
const unsigned long PM_SLEEP_DURATION = 120000;       // 2 mins sleep
const unsigned long BLYNK_SEND_INTERVAL = 60000;      // 1 min throttle
const unsigned long STABILITY_THRESHOLD = 30000;      // 30s stable wait

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
    Serial.println(">> MODE: Initial 5-minute warmup started.");
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
        if (pmSensor.readData(pm1_0_val, pm2_5_val, pm10_0_val)) {
            Serial.printf("[WARMUP] PM2.5: %.0f\n", pm2_5_val);
            rgbLEDHandler.updateLED(pm2_5_val, true);
            
            // Only allow data to be sent after first 15s of boot
            if (currentMillis - bootTime > STABILITY_THRESHOLD) {
                isDataFresh = true;
            }
        }

        if (currentMillis - bootTime >= INITIAL_WARMUP_DURATION) {
            isInitialWarmup = false;
            stateTimer = currentMillis;
            Serial.println(">> MODE: Warmup complete. Entering Duty Cycle.");
        }
    } 
    else if (sensorIsAwake) {
        if (pmSensor.readData(pm1_0_val, pm2_5_val, pm10_0_val)) {
            Serial.printf("[ACTIVE] PM2.5: %.0f\n", pm2_5_val);
            rgbLEDHandler.updateLED(pm2_5_val, true);

            // STABILITY LOCK: Only mark as fresh if fan has run for > 15s
            if (timeInState > STABILITY_THRESHOLD) {
                isDataFresh = true; 
            }
        }

        if (timeInState >= PM_WAKE_DURATION) {
            pmSensor.sleep();
            sensorIsAwake = false;
            stateTimer = currentMillis;
            Serial.println(">> SENSOR: Entering sleep (Duty Cycle).");
        }
    } 
    else {
        // While sleeping, we don't set isDataFresh to false.
        // This allows the stable reading captured at the end of the wake 
        // window to be uploaded if the Blynk timer expires during sleep.
        
        if (timeInState >= PM_SLEEP_DURATION) {
            Serial.println(">> SENSOR: Waking up for new cycle.");
            pmSensor.wakeup();
            pmSensor.clearBuffer();
            sensorIsAwake = true;
            stateTimer = currentMillis;
            
            // Reset freshness ONLY at wakeup so we don't send 
            // the first few unstable packets of the new cycle.
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

    // Criteria: 1. Timer expired, 2. WiFi connected, 3. Data is stabilized
    if (connected && (now - lastBlynkSend > BLYNK_SEND_INTERVAL) && isDataFresh) {
        Serial.println(">> BLYNK: Sending stable measurement.");
        blynkHandler.sendData(BLYNK_AUTH_TOKEN, pm1_0_val, pm2_5_val, pm10_0_val, temp_val, hum_val);
        lastBlynkSend = now;
        
        // Reset so we don't double-send the same packet in one window
        isDataFresh = false; 
    }
}
