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
unsigned long blynkFlashTimer = 0; // Timer for the 'B' icon

bool sensorAwake = true;
bool isDataFresh = false;
bool ledMuted = false;

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
}

void loop() {
    wifi.handleConnect();
    handleButton();
    handlePMSensor();
    
    data.temp = dhtSensor.readTemperature();
    data.hum = dhtSensor.readHumidity();
    
    updateUI();
    handleBlynkTransmission();

    // LED Logic
    if (ledMuted) {
        led.updateLED(0, false); // Off
    } else {
        // Show Blue if warming up/stabilizing, otherwise AQI color
        led.updateLED(data.pm2_5, isDataFresh || data.isWarmup);
    }

    delay(100); 
}

void handleButton() {
    static bool lastBtnState = HIGH;
    bool currentBtnState = digitalRead(BUTTON_PIN);
    
    if (lastBtnState == HIGH && currentBtnState == LOW) {
        ledMuted = !ledMuted;
        Serial.println(F(">> UI: LED Mute Toggled"));
        delay(200); 
    }
    lastBtnState = currentBtnState;
}

void handlePMSensor() {
    unsigned long now = millis();
    unsigned long elapsed = now - stateTimer;

    if (data.isWarmup) {
        if (pmSensor.readData(data.pm1_0, data.pm2_5, data.pm10_0)) {
            // During warmup, we allow the LED to show data but wait for stability
            if (now - bootTime > 30000) isDataFresh = true;
        }
        if (now - bootTime >= INITIAL_WARMUP_DURATION) {
            data.isWarmup = false; 
            stateTimer = now;
        }
    } else if (sensorAwake) {
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

    // FIX: Only show 'B' if we just uploaded data in the last 3 seconds
    data.showBlynkIcon = (now - blynkFlashTimer < BLYNK_ICON_KEEP_ALIVE);

    unsigned long elapsed = now - stateTimer;
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
    bool connected = (WiFi.status() == WL_CONNECTED);

    if (connected && isDataFresh && (now - lastBlynk > BLYNK_SEND_INTERVAL)) {
        Serial.println(F(">> BLYNK: Uploading stable data."));
        blynk.sendData(BLYNK_AUTH_TOKEN, data.pm1_0, data.pm2_5, data.pm10_0, data.temp, data.hum);
        
        lastBlynk = now;
        blynkFlashTimer = now; // Triggers the 'B' on OLED for 3 seconds
        isDataFresh = false; 
    }
}
