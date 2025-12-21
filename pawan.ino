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
RGBLEDHandler led(WS2812_PIN);
WiFiHandler wifi;
BlynkHandler blynk;

unsigned long lastBlynk = 0, stateTimer = 0, bootTime = 0, blynkTimer = 0;
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
    data.temp = dhtSensor.readTemperature();
    data.hum = dhtSensor.readHumidity();
    updateUI();
    handleBlynk();
    delay(100);
}

void handleButton() {
    if (digitalRead(BUTTON_PIN) == LOW) {
        data.showBlynkIcon = !data.showBlynkIcon; // Example toggle
        delay(300); // Simple debounce
    }
}

void handlePMSensor() {
    unsigned long now = millis();
    if (data.isWarmup) {
        if (pmSensor.readData(data.pm1_0, data.pm2_5, data.pm10_0)) {
            if (now - bootTime > 30000) isDataFresh = true;
        }
        if (now - bootTime >= INITIAL_WARMUP_DURATION) {
            data.isWarmup = false; stateTimer = now;
        }
    } else if (sensorAwake) {
        if (pmSensor.readData(data.pm1_0, data.pm2_5, data.pm10_0)) {
            if (now - stateTimer > STABILITY_THRESHOLD) isDataFresh = true;
        }
        if (now - stateTimer >= PM_WAKE_DURATION) {
            pmSensor.sleep(); sensorAwake = false; stateTimer = now; isDataFresh = false;
        }
    } else if (now - stateTimer >= PM_SLEEP_DURATION) {
        pmSensor.wakeup(); sensorAwake = true; stateTimer = now;
    }
    led.updateLED(data.pm2_5, isDataFresh || data.isWarmup);
}

void updateUI() {
    data.wifiStatus = wifi.getWifiStatus();
    data.isSleeping = !sensorAwake;
    unsigned long elapsed = millis() - stateTimer;
    if (data.isWarmup) data.countdown = (INITIAL_WARMUP_DURATION - (millis() - bootTime)) / 1000;
    else data.countdown = ((sensorAwake ? PM_WAKE_DURATION : PM_SLEEP_DURATION) - elapsed) / 1000;
    oled.update(data);
}

void handleBlynk() {
    if (isDataFresh && (millis() - lastBlynk > BLYNK_SEND_INTERVAL)) {
        blynk.sendData(BLYNK_AUTH_TOKEN, data.pm1_0, data.pm2_5, data.pm10_0, data.temp, data.hum);
        lastBlynk = millis();
    }
}
