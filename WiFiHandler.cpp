// WiFiHandler.cpp

#include "WiFiHandler.h"
#include <ESP8266WiFi.h>

// Assuming WiFiHandler.h defines the enum:
// enum ConnectionMode { IDLE, STA_CONNECTING, AP_CONFIG_PORTAL };
// and includes WiFiManager wifiManager;

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

    // Always start in STA mode and call WiFi.begin() to load saved credentials if any
    WiFi.mode(WIFI_STA);
    WiFi.begin(); 

    // Check if credentials were loaded (WiFi.SSID() will be empty if not)
    if (WiFi.SSID().length() == 0) {
        // Case #1: No saved config. Go straight to non-blocking AP mode.
        Serial.println("Case #1: No saved config. Starting Config AP...");
        // This initiates the AP creation
        wifiManager.startConfigPortal("airmon_AP", "password"); 
        _connectMode = AP_CONFIG_PORTAL;
        _apModeStartTime = millis();
    } else {
        // Case #2: Saved config exists. Attempt quick connect.
        Serial.print("Attempting quick connect with saved credentials (");
        Serial.print(_quickConnectTimeoutMs / 1000);
        Serial.println("s timeout). SSID: " + WiFi.SSID());

        _connectStartTime = millis();
        _connectMode = STA_CONNECTING;
    }
}

bool WiFiHandler::handleConnect() {
    // 1. Check for a successful STA connection (not AP mode)
    if (WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_STA) {
        if (_connectMode != IDLE) {
            Serial.println("\nSUCCESS! Wi-Fi Connected.");
            _connectMode = IDLE; // Transition to IDLE once connected
        }
        return true;
    }

    // 2. Handle different connection modes
    switch (_connectMode) {
        case STA_CONNECTING:
            if ((millis() - _connectStartTime) < _quickConnectTimeoutMs) {
                // Still within STA quick connect timeout, keep trying
                return false; 
            } else {
                // STA quick connect timed out, fallback to AP mode
                Serial.println("\nSTA quick connect timed out. Starting Config AP fallback.");
                WiFi.disconnect(); // Disconnect STA to cleanly start AP
                
                // Switch to AP_STA mode for config portal
                WiFi.mode(WIFI_AP_STA); 
                wifiManager.startConfigPortal("airmon_AP", "password");
                
                _connectMode = AP_CONFIG_PORTAL;
                _apModeStartTime = millis();
                return false; // Not connected yet, AP mode initiated.
            }
        case AP_CONFIG_PORTAL:
            // CRITICAL: Keep the config portal running every loop cycle
            wifiManager.process(); 

            if ((millis() - _apModeStartTime) < _apModeTimeoutSec * 1000UL) {
                // Still within AP mode timeout
                return false;
            } else {
                // AP mode timed out, connection failed through portal
                Serial.println("\nAP Configuration Portal timed out. Connection failed.");
                
                // Switch back to STA mode, but disconnected, and stop trying.
                WiFi.mode(WIFI_STA); 
                _connectMode = IDLE; 
                return false;
            }
        case IDLE:
            // If in IDLE and not connected (WiFi.status() != WL_CONNECTED),
            // we are simply waiting for startConnect to be called again, or we failed/timed out previously.
            return false;
    }
    return false; // Should not be reached
}


String WiFiHandler::getWifiStatus() {
    // Check if connected as a Station (client)
    if (WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_STA) {
        return "Connected";
    }

    switch (_connectMode) {
        case IDLE:
            return "Idle/Disconnected";
        case STA_CONNECTING:
            return "Connecting...";
        case AP_CONFIG_PORTAL:
            // This is the status for the display when the AP is active
            return "AP Config"; 
    }
    return "Unknown"; 
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
