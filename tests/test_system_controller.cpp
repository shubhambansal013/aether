#include "Arduino.h"
#include <vector>
#include <assert.h>
#include <iostream>

#include "EEPROM.h"
#include "ESP8266WiFi.h"
#include "ESP8266httpUpdate.h"

// System components mocks
#include "PMSensorMock.h"
#include "DHTSensorMock.h"
#include "OLEDDisplayMock.h"
#include "RGBLEDHandlerMock.h"
#include "WiFiHandlerMock.h"
#include "BlynkHandlerMock.h"
#include "ButtonHandlerMock.h"

unsigned long mock_millis = 0;
unsigned long millis() { return mock_millis; }
void delay(unsigned long ms) { mock_millis += ms; }
void pinMode(int pin, int mode) {}
void digitalWrite(int pin, int val) {}

SerialMock Serial;
WiFiMock WiFi;
EEPROMMock EEPROM;
ESP8266HTTPUpdate ESPhttpUpdate;

#include "../SystemController.h"

void test_initial_state() {
    std::cout << "Running test_initial_state..." << std::endl;
    PMSensor pm(0,0); DHTSensor dht(0); OLEDDisplay oled(0,0);
    RGBLEDHandler led(0); WiFiHandler wifi; BlynkHandler blynk; ButtonHandler button(0);
    SystemController sc(pm, dht, oled, led, wifi, blynk, button);

    // Seed EEPROM to test initialization
    byte mode = (byte)MODE_ACTIVE;
    byte muted = 1; // true
    EEPROM.put(ADDR_MODE, mode);
    EEPROM.put(ADDR_MUTED, muted);

    sc.setup();

    assert(sc.getData().currentMode == MODE_ACTIVE);
    assert(sc.isMuted() == true);
}

void test_mode_cycling() {
    std::cout << "Running test_mode_cycling..." << std::endl;
    PMSensor pm(0,0); DHTSensor dht(0); OLEDDisplay oled(0,0);
    RGBLEDHandler led(0); WiFiHandler wifi; BlynkHandler blynk; ButtonHandler button(0);
    SystemController sc(pm, dht, oled, led, wifi, blynk, button);

    sc.setup(); // Default is MODE_PASSIVE if not in EEPROM, or from DEFAULT_MODE_SETTING

    // Set to ACTIVE
    if (sc.getData().currentMode != MODE_ACTIVE) sc.cycleSystemMode();
    assert(sc.getData().currentMode == MODE_ACTIVE);

    sc.cycleSystemMode();
    assert(sc.getData().currentMode == MODE_PASSIVE);
}

void test_stealth_mode_suppression() {
    std::cout << "Running test_stealth_mode_suppression..." << std::endl;
    PMSensor pm(0,0); DHTSensor dht(0); OLEDDisplay oled(0,0);
    RGBLEDHandler led(0); WiFiHandler wifi; BlynkHandler blynk; ButtonHandler button(0);
    SystemController sc(pm, dht, oled, led, wifi, blynk, button);

    sc.setup();
    // Ensure not muted
    if (sc.isMuted()) sc.toggleStealthMode();

    oled.updateCount = 0;
    led.updateCount = 0;

    sc.update();
    assert(oled.updateCount > 0);
    assert(led.updateCount > 0);

    sc.toggleStealthMode();
    assert(sc.isMuted() == true);

    oled.updateCount = 0;
    led.updateCount = 0;
    oled.clearCount = 0;
    led.turnOffCount = 0;

    sc.update();
    assert(oled.updateCount == 0);
    assert(led.updateCount == 0);
    assert(oled.clearCount > 0);
    assert(led.turnOffCount > 0);
}

void test_passive_mode_cycle() {
    std::cout << "Running test_passive_mode_cycle..." << std::endl;
    PMSensor pm(0,0); DHTSensor dht(0); OLEDDisplay oled(0,0);
    RGBLEDHandler led(0); WiFiHandler wifi; BlynkHandler blynk; ButtonHandler button(0);
    SystemController sc(pm, dht, oled, led, wifi, blynk, button);

    EEPROM.put(ADDR_MODE, (byte)MODE_PASSIVE);
    mock_millis = 1000;
    sc.setup();

    assert(sc.getData().currentMode == MODE_PASSIVE);
    assert(pm.sleepCount == 0);

    // Stay awake for PM_WAKE_DURATION (32s)
    mock_millis += 31000;
    sc.update();
    assert(pm.sleepCount == 0);

    mock_millis += 2000; // Total 33s since setup
    sc.update();
    assert(pm.sleepCount == 1); // Should have called sleep

    // Stay asleep for PM_SLEEP_DURATION (60s)
    mock_millis += 59000;
    sc.update();
    assert(pm.wakeupCount == 1); // Initial wakeup in setup is 1

    assert(pm.wakeupCount == 1);
    mock_millis += 2000;
    sc.update();
    assert(pm.wakeupCount == 2);
}

void test_blynk_transmission_active() {
    std::cout << "Running test_blynk_transmission_active..." << std::endl;
    PMSensor pm(0,0); DHTSensor dht(0); OLEDDisplay oled(0,0);
    RGBLEDHandler led(0); WiFiHandler wifi; BlynkHandler blynk; ButtonHandler button(0);
    SystemController sc(pm, dht, oled, led, wifi, blynk, button);

    EEPROM.put(ADDR_MODE, (byte)MODE_ACTIVE);
    mock_millis = 100000;
    sc.setup();

    blynk.sendCount = 0;
    pm.mock_readData_return = true;
    sc.update();
    assert(blynk.sendCount == 1);

    mock_millis += 30000;
    sc.update();
    assert(blynk.sendCount == 1);

    mock_millis += 31000;
    sc.update();
    assert(blynk.sendCount == 2);
}

int main() {
    test_initial_state();
    test_mode_cycling();
    test_stealth_mode_suppression();
    test_passive_mode_cycle();
    test_blynk_transmission_active();
    std::cout << "All SystemController tests passed!" << std::endl;
    return 0;
}
