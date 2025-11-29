// WiFiHandler.cpp

#include "WiFiHandler.h"
#include <ESP8266WiFi.h>

// --- Public Methods ---

void WiFiHandler::startConnect(unsigned long quickConnectTimeoutMs, unsigned long apModeTimeoutSec) {
    _quickConnectTimeoutMs = quickConnectTimeoutMs;
    _apModeTimeoutSec = apModeTimeoutSec;
    _connectMode = IDLE; // Reset initial state

    // NOTE: These use the static callback functions defined below
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.setAPCallback(configModeCallback);
    
    // Set the connect timeout for STA mode attempts (used by WiFiManager internally)
    wifiManager.setConnectTimeout(_quickConnectTimeoutMs); 

    const char* savedSsid = wifiManager.getWiFiSSID().c_str();

    if (savedSsid[0] == '\0') {
        // Case #1: No saved config. Go straight to non-blocking AP mode.
        Serial.println("Case #1: No saved config. Starting Config AP...");
        wifiManager.startConfigPortal("airmon_AP", "password");
        _connectMode = AP_CONFIG_PORTAL;
        _apModeStartTime = millis();
    } else {
        // Case #2: Saved config exists. Attempt quick connect.
        Serial.print("Attempting quick connect with saved credentials (");
        Serial.print(_quickConnectTimeoutMs / 1000);
        Serial.println("s timeout)...");

        WiFi.mode(WIFI_STA);
        WiFi.begin();
        _connectStartTime = millis();
        _connectMode = STA_CONNECTING;
    }
}

bool WiFiHandler::handleConnect() {
    // If already connected, just return true.
    if (WiFi.status() == WL_CONNECTED) {
        if (_connectMode != IDLE) {
            Serial.println("\nSUCCESS! Wi-Fi Connected.");
            _connectMode = IDLE; // Transition to IDLE once connected
        }
        return true;
    }

    // Handle different connection modes
    switch (_connectMode) {
        case STA_CONNECTING:
            if ((millis() - _connectStartTime) < _quickConnectTimeoutMs) {
                // Still within STA quick connect timeout, keep trying
                return false;
            } else {
                // STA quick connect timed out, fallback to AP mode
                Serial.println("\nSTA quick connect timed out. Starting Config AP fallback.");
                WiFi.mode(WIFI_AP_STA); // Switch to AP_STA mode for config portal
                wifiManager.startConfigPortal("airmon_AP", "password");
                _connectMode = AP_CONFIG_PORTAL;
                _apModeStartTime = millis();
                return false; // Not connected yet, AP mode initiated.
            }
        case AP_CONFIG_PORTAL:
            wifiManager.process(); // Keep the config portal running

            if ((millis() - _apModeStartTime) < _apModeTimeoutSec * 1000UL) {
                // Still within AP mode timeout
                return false;
            } else {
                // AP mode timed out, connection failed through portal
                Serial.println("\nAP Configuration Portal timed out. Connection failed.");
                _connectMode = IDLE; // No longer actively trying to connect via AP
                // Optionally, trigger a reboot or further action in main loop
                return false;
            }
        case IDLE:
            // If in IDLE and not connected, something went wrong or waiting for startConnect.
            return false;
    }
    return false; // Should not be reached
}


String WiFiHandler::getWifiStatus() {
    switch (_connectMode) {
        case IDLE:
            return (WiFi.status() == WL_CONNECTED) ? "Connected" : "Idle";
        case STA_CONNECTING:
            return "Connecting...";
        case AP_CONFIG_PORTAL:
            return "AP Mode";
    }
    return "Unknown"; // Should not be reached
}

void WiFiHandler::resetSettings() {
    Serial.println("Clearing Wi-Fi credentials via WiFiHandler.");
    wifiManager.resetSettings();
}

// --- Private Static Callback Implementations ---

void WiFiHandler::saveConfigCallback () {
    Serial.println("Wi-Fi credentials saved to EEPROM.");
}

void WiFiHandler::configModeCallback (WiFiManager *myWiFiManager) {
    Serial.println("Entered Configuration Mode (Connect to AP airmon_AP).");
    Serial.print("Config Portal IP: ");
    Serial.println(WiFi.softAPIP());
}
