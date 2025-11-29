// WiFiHandler.cpp

#include "WiFiHandler.h"
#include <ESP8266WiFi.h>

// --- Public Methods ---

void WiFiHandler::startConnect(unsigned long quickConnectTimeoutMs, int apTimeoutSec) {
    _quickConnectTimeoutMs = quickConnectTimeoutMs;
    _apTimeoutSec = apTimeoutSec;
    _connectionAttemptInProgress = false;
    _apModeStarted = false;

    // NOTE: This uses the static callback functions defined below
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.setAPCallback(configModeCallback);

    const char* savedSsid = wifiManager.getWiFiSSID().c_str();

    // Path 1: Case #1 - No saved config: Go straight to AP mode.
    if (savedSsid[0] == '\0') {
        Serial.println("Case #1: No saved config. Starting Config AP...");
        // wifiManager.setConnectTimeout(_apTimeoutSec); // This timeout is for STA mode connection attempts, not AP mode duration.
        // For AP mode duration, it's better to let autoConnect manage it or implement custom timeout logic in handleConnect
        _apModeStarted = true;
        wifiManager.autoConnect("airmon_AP", "password"); // This is blocking, we need to handle it differently
        // For non-blocking AP, we'd typically run it in a separate task or check status frequently.
        // For now, assume autoConnect eventually returns or we restart.
    } else {
        // Path 2: Saved config exists. Attempt quick connect.
        Serial.print("Attempting quick connect with saved credentials (");
        Serial.print(_quickConnectTimeoutMs / 1000);
        Serial.println("s timeout)...");

        WiFi.mode(WIFI_STA);
        WiFi.begin();
        _connectStartTime = millis();
        _connectionAttemptInProgress = true;
    }
}

bool WiFiHandler::handleConnect() {
    if (WiFi.status() == WL_CONNECTED) {
        if (_connectionAttemptInProgress) {
            Serial.println("\nSUCCESS! Connected with saved credentials.");
            _connectionAttemptInProgress = false; // Connection established
        }
        return true;
    }

    if (_apModeStarted) {
        // If AP mode was started, we assume autoConnect is handling it.
        // This part might need refinement if a truly non-blocking AP mode with timeout is desired.
        // For now, if we are in AP mode, we consider ourselves "connected" to the portal.
        return false; // Not connected to external WiFi, but AP is active.
    }

    if (_connectionAttemptInProgress) {
        if ((millis() - _connectStartTime) < _quickConnectTimeoutMs) {
            // Still within timeout, keep trying
            return false;
        } else {
            // Quick connect timed out, fallback to AP mode
            Serial.println("\nQuick connect timed out. Starting Config AP fallback.");
            _connectionAttemptInProgress = false;
            _apModeStarted = true;
            wifiManager.setConnectTimeout(_apTimeoutSec); // This timeout is for STA mode connection *attempts* when in AP mode
            wifiManager.autoConnect("airmon_AP", "password");
            // If autoConnect returns false, it means AP mode also failed or timed out.
            // In a real non-blocking scenario, we'd check its status in subsequent loops.
            return false; // Not connected, AP mode initiated.
        }
    }
    return false; // Not connected, and no active connection attempt.
}


String WiFiHandler::getWifiStatus() {
    switch (WiFi.status()) {
        case WL_IDLE_STATUS: return "Idle";
        case WL_NO_SSID_AVAIL: return "No SSID";
        case WL_SCAN_COMPLETED: return "Scan done";
        case WL_CONNECTED: return "Connected";
        case WL_CONNECT_FAILED: return "Conn Failed";
        case WL_CONNECTION_LOST: return "Conn Lost";
        case WL_DISCONNECTED: return "Disconn.";
        default: return "Unknown";
    }
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