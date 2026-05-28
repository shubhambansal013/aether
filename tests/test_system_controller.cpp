#include "Arduino.h"
#include "EEPROM.h"
#include "ESP8266WiFi.h"
#include "ESP8266httpUpdate.h"
#include <vector>

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
#include <assert.h>

// Minimal mocks for other classes to make it compile and testable
// We might need to mock them more if we want to test interactions

void PMSensor::begin(long b) {}
void PMSensor::sleep() {}
void PMSensor::wakeup() {}
bool PMSensor::readData(float& pm1, float& pm2, float& pm10) { pm1 = 1.0; pm2 = 2.0; pm10 = 3.0; return true; }
PMSensor::PMSensor(int r, int s) : _pmSerial(r, s), _setPin(s) {}

void DHTSensor::setup() {}
float DHTSensor::getTemperature() { return 25.0; }
float DHTSensor::getHumidity() { return 50.0; }
DHTSensor::DHTSensor(int p) {}

void OLEDDisplay::setup() {}
void OLEDDisplay::update(const SystemData& d) {}
void OLEDDisplay::clear() {}
void OLEDDisplay::printMessage(String l1, String l2) {}
OLEDDisplay::OLEDDisplay(int sda, int scl) {}

void RGBLEDHandler::setup() {}
void RGBLEDHandler::updateLED(float pm) {}
void RGBLEDHandler::turnOff() {}
void RGBLEDHandler::setColor(uint32_t hex) {}
void RGBLEDHandler::startupSequence() {}
RGBLEDHandler::RGBLEDHandler(int p) : _strip(1, p, 0) {}

void WiFiHandler::startConnect() {}
bool WiFiHandler::handleConnect() { return true; }
String WiFiHandler::getWifiStatus() { return "Connected"; }
void WiFiHandler::resetSettings() {}

void BlynkHandler::sendData(const char* a, float p1, float p2, float p10, float t, float h) {}
BlynkHandler::BlynkHandler() {}

void ButtonHandler::setup() {}
bool ButtonHandler::isPressed() { return false; }
bool ButtonHandler::isLongPressed() { return false; }
ButtonHandler::ButtonHandler(int p) {}

// SoftwareSerial mock if needed
#include <SoftwareSerial.h>
SoftwareSerial::SoftwareSerial(int rx, int tx) {}
void SoftwareSerial::begin(long b) {}

void test_initial_state() {
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
    PMSensor pm(0,0); DHTSensor dht(0); OLEDDisplay oled(0,0);
    RGBLEDHandler led(0); WiFiHandler wifi; BlynkHandler blynk; ButtonHandler button(0);
    SystemController sc(pm, dht, oled, led, wifi, blynk, button);

    sc.cycleSystemMode();
    assert(sc.getData().currentMode == MODE_ACTIVE);
    sc.cycleSystemMode();
    assert(sc.getData().currentMode == MODE_PASSIVE);
}

void test_stealth_mode() {
    PMSensor pm(0,0); DHTSensor dht(0); OLEDDisplay oled(0,0);
    RGBLEDHandler led(0); WiFiHandler wifi; BlynkHandler blynk; ButtonHandler button(0);
    SystemController sc(pm, dht, oled, led, wifi, blynk, button);

    assert(sc.isMuted() == false);
    sc.toggleStealthMode();
    assert(sc.isMuted() == true);
    sc.toggleStealthMode();
    assert(sc.isMuted() == false);
}

int main() {
    test_initial_state();
    test_mode_cycling();
    test_stealth_mode();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
