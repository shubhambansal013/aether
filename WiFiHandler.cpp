// WiFiHandler.cpp

#include "WiFiHandler.h"
#include <ESP8266WiFi.h>

// Helper function (defined in this translation unit, no class scope needed)
const int MAX_SCAN_ATTEMPTS = 5; 

bool isSSIDAvailable(const char* targetSsid) {
    if (targetSsid == nullptr || targetSsid[0] == '\0') {
        return false;
    }
    
    Serial.println("Starting multiple scan attempts for saved SSID...");
    
    for (int attempt = 1; attempt <= MAX_SCAN_ATTEMPTS; ++attempt) {
        Serial.print("Scan Attempt #");
        Serial.print(attempt);
        Serial.print("...");
        
        int n = WiFi.scanNetworks(); 
        
        if (n > 0) {
            for (int i = 0; i < n; ++i) {
                if (WiFi.SSID(i) == String(targetSsid)) {
                    Serial.println("SUCCESS! SSID found.");
                    return true;
                }
            }
        }
        
        Serial.println("Not found. Waiting 2s.");
        delay(2000); 
    }
    
    Serial.print("FAILURE: Saved SSID [");
    Serial.print(targetSsid);
    Serial.println("] not found after all attempts.");
    return false;
}

// --- Public Methods ---

bool WiFiHandler::connect(unsigned long quickConnectTimeoutMs, int apTimeoutSec) {
    
    // NOTE: This uses the static callback functions defined below
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.setAPCallback(configModeCallback);
    
    const char* savedSsid = wifiManager.getWiFiSSID().c_str();

    // Path 1: Case #1 - No saved config: Go straight to AP mode.
    if (savedSsid[0] == '\0') {
        Serial.println("Case #1: No saved config. Starting Config AP...");
        wifiManager.setConnectTimeout(apTimeoutSec); 
        return wifiManager.autoConnect("airmon_AP", "password");
    }

    // Path 2 & 3 Logic
    if (isSSIDAvailable(savedSsid)) {
        Serial.print("Case #3: Saved SSID is available. Attempting connect...");
        
        WiFi.mode(WIFI_STA); 
        WiFi.begin();
        
        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < quickConnectTimeoutMs) {
            delay(500);
            Serial.print(".");
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nSUCCESS! Connected with saved credentials.");
            return true; 
        }

        Serial.println("\nCase #3 Failed: Connection attempt timed out or failed.");
    }
    
    // Path 3: Case #2 & Failed Case #3 - Fallback to AP Mode.
    Serial.println("Starting Config AP fallback. User intervention required.");
    
    wifiManager.setConnectTimeout(apTimeoutSec); 
    
    return wifiManager.autoConnect("airmon_AP", "password");
}

void WiFiHandler::resetSettings() { // <<<--- FIX: Correct Definition for ResetHandler
    Serial.println("Clearing Wi-Fi credentials via WiFiHandler.");
    wifiManager.resetSettings();
}

// --- Private Static Callback Implementations ---

void WiFiHandler::saveConfigCallback () { // <<<--- CRITICAL FIX: Added WiFiHandler::
    Serial.println("Wi-Fi credentials saved to EEPROM.");
}

void WiFiHandler::configModeCallback (WiFiManager *myWiFiManager) { // <<<--- CRITICAL FIX: Added WiFiHandler::
    Serial.println("Entered Configuration Mode (Connect to AP airmon_AP).");
    Serial.print("Config Portal IP: ");
    Serial.println(WiFi.softAPIP());
    
}
