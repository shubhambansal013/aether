#include "Arduino.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstdarg>

std::vector<std::string> serial_logs;
int wifi_status = WL_IDLE_STATUS;
int mock_http_code = 200;
String mock_payload = "{\"version\":\"1.1.0\",\"url\":\"http://example.com/fw.bin\"}";

#include "ESP8266httpUpdate.h"
t_httpUpdate_return mock_update_return = HTTP_UPDATE_OK;

#include "OTAHandler.h"
#include <assert.h>

// Other mocks
void SerialMock::printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buf[256];
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    print(buf);
}

unsigned long mock_millis = 0;
unsigned long millis() { return mock_millis; }
void delay(unsigned long ms) { mock_millis += ms; }
void pinMode(int pin, int mode) {}
void digitalWrite(int pin, int val) {}

SerialMock Serial;
WiFiMock WiFi;
ESP8266HTTPUpdate ESPhttpUpdate;

void OLEDDisplay::printMessage(String l1, String l2) {}
OLEDDisplay::OLEDDisplay(int sda, int scl) {}
void RGBLEDHandler::setColor(uint32_t hex) {}
RGBLEDHandler::RGBLEDHandler(int p) : _strip(1, p, 0) {}

void test_ota_workflow() {
    serial_logs.clear();
    mock_millis = 0;
    wifi_status = WL_IDLE_STATUS;

    OTAHandler ota;
    ota.setup(nullptr, nullptr);

    // 1. Handle when disconnected
    std::cout << "Step 1: Disconnected" << std::endl;
    ota.handle();
    assert(serial_logs.empty());

    // 2. Advance time but still disconnected
    std::cout << "Step 2: Disconnected after 31s" << std::endl;
    mock_millis = 31000;
    ota.handle();
    bool found_disconnected_log = false;
    for(auto& log : serial_logs) {
        std::cout << "LOG: " << log;
        if (log.find("OTA: WiFi not connected") != std::string::npos) found_disconnected_log = true;
    }
    assert(found_disconnected_log);

    // 3. Connect WiFi
    std::cout << "Step 3: Connected" << std::endl;
    wifi_status = WL_CONNECTED;
    serial_logs.clear();

    // We need to trigger the check. Since _lastCheck was set to 31000,
    // and OTA_CHECK_INTERVAL is 60000, we need mock_millis to be at least 31000 + 60000.
    mock_millis = 100000;
    ota.handle();

    bool found_checking_log = false;
    bool found_new_version_log = false;
    bool found_success_log = false;

    for(auto& log : serial_logs) {
        std::cout << "LOG: " << log;
        if (log.find("Checking for updates...") != std::string::npos) found_checking_log = true;
        if (log.find("New version available: 1.1.0") != std::string::npos) found_new_version_log = true;
        if (log.find("OTA update successful!") != std::string::npos) found_success_log = true;
    }
    assert(found_checking_log);
    assert(found_new_version_log);
    assert(found_success_log);
}

int main() {
    test_ota_workflow();
    std::cout << "OTA logic tests passed!" << std::endl;
    return 0;
}
