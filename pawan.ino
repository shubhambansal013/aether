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

SystemData data;
PMSensor pmSensor(PM_SENSOR_RX_PIN, PM_SENSOR_SET_PIN);
DHTSensor dhtSensor(DHT_PIN);
OLEDDisplay oled(OLED_SDA_PIN, OLED_SCL_PIN);
RGBLEDHandler led(WS2812_PIN); // Pin 15 (D8)
WiFiHandler wifi;
BlynkHandler blynk;

unsigned long lastBlynk = 0, stateTimer = 0, bootTime = 0, blynkFlashTimer = 0;
bool sensorAwake = true, isDataFresh = false;

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
    
    data.temp = dhtSensor.getTemperature();
    data.hum  = dhtSensor.getHumidity();
    
    updateUI();
    handleBlynkTransmission();

    // LED always follows data.pm2_5 now
    led.updateLED(data.pm2_5); 

    delay(100); 
}

void handleButton() {
    // Logic removed. Button is currently free for other uses.
}

void handlePMSensor() {
    unsigned long now = millis();
    unsigned long elapsed = now - stateTimer;

    if (data.isWarmup || sensorAwake) {
        if (pmSensor.readData(data.pm1_0, data.pm2_5, data.pm10_0)) {
            // Log to Serial to confirm the sensor is talking
            if (millis() % 2000 < 100) Serial.printf("Sensor Data: %.0f\n", data.pm2_5);

            if (data.isWarmup) {
                if (now - bootTime > STABILITY_THRESHOLD) isDataFresh = true;
            } else {
                if (elapsed > STABILITY_THRESHOLD) isDataFresh = true;
            }
        }
        
        if (data.isWarmup && (now - bootTime >= INITIAL_WARMUP_DURATION)) {
            data.isWarmup = false; stateTimer = now;
        } else if (!data.isWarmup && (elapsed >= PM_WAKE_DURATION)) {
            pmSensor.sleep(); sensorAwake = false; stateTimer = now; isDataFresh = false;
        }
    } else if (now - stateTimer >= PM_SLEEP_DURATION) {
        pmSensor.wakeup(); sensorAwake = true; stateTimer = now;
    }
}

void updateUI() {
    unsigned long now = millis();
    data.wifiStatus = wifi.getWifiStatus();
    data.isSleeping = !sensorAwake;
    data.showBlynkIcon = (now - blynkFlashTimer < BLYNK_ICON_KEEP_ALIVE);

    unsigned long elapsed = now - stateTimer;
    if (data.isWarmup) data.countdown = (INITIAL_WARMUP_DURATION - (now - bootTime)) / 1000;
    else data.countdown = ((sensorAwake ? PM_WAKE_DURATION : PM_SLEEP_DURATION) - elapsed) / 1000;
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
