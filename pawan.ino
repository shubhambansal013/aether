#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include "Config.h"       // Unified Pins and Timers
#include "SystemData.h"   // Data Transfer Object
#include "WiFiHandler.h"
#include "ResetHandler.h"
#include "PMSensor.h"
#include "BlynkHandler.h"
#include "blynk_config.h"
#include "DHTSensor.h"
#include "OLEDDisplay.h"
#include "RGBLEDHandler.h"

// --- Global Instances ---
WiFiHandler wifiHandler;
ResetHandler resetHandler(wifiHandler);
PMSensor pmSensor(PM_SENSOR_RX_PIN, PM_SENSOR_SET_PIN); 
BlynkHandler blynkHandler;

// PASS PIN FROM CONFIG TO SENSOR HERE
DHTSensor dhtSensor(DHT_PIN); 
OLEDDisplay oledDisplay(OLED_SDA_PIN, OLED_SCL_PIN);
RGBLEDHandler rgbLEDHandler(RGB_LED_RED_PIN, RGB_LED_GREEN_PIN, RGB_LED_BLUE_PIN);

// --- Global State ---
SystemData currentData; 
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
// 🛠️ PM SENSOR STATE MACHINE
// ----------------------------------------------------------------------

void handlePMSensor() {
    if (isInitialWarmup) {
        runInitialWarmup();
    } else if (sensorIsAwake) {
        runAwakeCycle();
    } else {
        runSleepCycle();
    }
}

void runInitialWarmup() {
    unsigned long currentMillis = millis();
    
    if (pmSensor.readData(currentData.pm1_0, currentData.pm2_5, currentData.pm10_0)) {
        rgbLEDHandler.updateLED(currentData.pm2_5, true);
        if (currentMillis - bootTime > 60000) {
            isDataFresh = true;
        }
    }

    if (currentMillis - bootTime >= INITIAL_WARMUP_DURATION) {
        isInitialWarmup = false;
        stateTimer = currentMillis;
        Serial.println(F(">> State: Initial Warmup Finished."));
    }
}

void runAwakeCycle() {
    unsigned long timeInState = millis() - stateTimer;

    if (pmSensor.readData(currentData.pm1_0, currentData.pm2_5, currentData.pm10_0)) {
        rgbLEDHandler.updateLED(currentData.pm2_5, true);
        if (timeInState > STABILITY_THRESHOLD) {
            isDataFresh = true; 
        }
    }

    if (timeInState >= PM_WAKE_DURATION) {
        pmSensor.sleep();
        sensorIsAwake = false;
        stateTimer = millis();
        isDataFresh = false; 
        Serial.println(F(">> State: Target Met. Sensor Hibernating."));
    }
}

void runSleepCycle() {
    unsigned long timeInState = millis() - stateTimer;

    if (timeInState >= PM_SLEEP_DURATION) {
        Serial.println(F(">> State: Cycle Triggered. Waking Sensor."));
        pmSensor.wakeup();
        pmSensor.clearBuffer();
        sensorIsAwake = true;
        stateTimer = millis();
    }
}

// ----------------------------------------------------------------------
// 🛠️ SUPPORT METHODS
// ----------------------------------------------------------------------

void updateSecondarySensors() {
    currentData.hum = dhtSensor.readHumidity();
    currentData.temp = dhtSensor.readTemperature();
}

void updateDisplays() {
    unsigned long now = millis();
    
    currentData.wifiStatus = wifiHandler.getWifiStatus();
    currentData.isWarmup = isInitialWarmup;
    currentData.isSleeping = !sensorIsAwake;
    currentData.showBlynkIcon = (now - blynkIconTimer < BLYNK_ICON_KEEP_ALIVE);

    if (isInitialWarmup) 
        currentData.countdown = (INITIAL_WARMUP_DURATION - (now - bootTime)) / 1000;
    else if (sensorIsAwake) 
        currentData.countdown = (PM_WAKE_DURATION - (now - stateTimer)) / 1000;
    else 
        currentData.countdown = (PM_SLEEP_DURATION - (now - stateTimer)) / 1000;

    if (currentData.countdown < 0) currentData.countdown = 0;

    oledDisplay.update(currentData);
}

void handleCloudUpdates() {
    unsigned long now = millis();
    bool connected = (WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_STA);

    if (connected && (now - lastBlynkSend > BLYNK_SEND_INTERVAL) && isDataFresh) {
        blynkHandler.sendData(BLYNK_AUTH_TOKEN, 
                              currentData.pm1_0, currentData.pm2_5, currentData.pm10_0, 
                              currentData.temp, currentData.hum);
        
        lastBlynkSend = now;
        isDataFresh = false; 
        blynkIconTimer = now; 
    }
}
