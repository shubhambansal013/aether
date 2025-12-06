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
    
    // Set the connect timeout for STA mode attempts
    wifiManager.setConnectTimeout(_quickConnectTimeoutMs);  

    // Always start in STA mode and call WiFi.begin() to load saved credentials if any.
    // This function call is non-blocking.
    WiFi.mode(WIFI_STA);
    WiFi.begin(); 

    // The startConnect function only sets the state to STA_CONNECTING and returns immediately.
    // All subsequent connection logic, including the AP fallback, is handled in handleConnect().
    
    Serial.print("Attempting quick connect (Max ");
    Serial.print(_quickConnectTimeoutMs / 1000);
    Serial.println("s timeout).");

    _connectStartTime = millis();
    _connectMode = STA_CONNECTING;
}

bool WiFiHandler::handleConnect() {
    // 1. Check for a successful STA connection (not AP mode)
    if (WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_STA) {
        if (_connectMode != IDLE) {
            Serial.println("\nSUCCESS! Wi-Fi Connected.");
            _connectMode = IDLE; 
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
                // *** CRITICAL: STA connect timed out, fallback to AP mode ***
                Serial.println("\nSTA connect timed out. STARTING NON-BLOCKING CONFIG AP.");
                
                WiFi.disconnect(); // Cleanly disconnect STA
                
                // Switch to AP_STA mode for config portal
                WiFi.mode(WIFI_AP_STA); 
                
                // *** AP portal is started here, non-blocking, deep inside the loop() ***
                wifiManager.startConfigPortal("airmon_AP", "password");
                
                _connectMode = AP_CONFIG_PORTAL;
                _apModeStartTime = millis();
                return false; 
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
                WiFi.mode(WIFI_STA); // Switch back to disconnected STA mode
                _connectMode = IDLE; 
                return false;
            }
            
        case IDLE:
            return false;
    }
    return false; 
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
