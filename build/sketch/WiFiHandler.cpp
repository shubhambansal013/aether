#line 1 "/home/runner/work/aether/aether/WiFiHandler.cpp"
// WiFiHandler.cpp
#include "WiFiHandler.h"
#include <ESP8266WiFi.h>

// Define static IP configuration for the Soft AP
const IPAddress AP_LOCAL_IP(192, 168, 4, 1);
const IPAddress AP_GATEWAY(192, 168, 4, 1);
const IPAddress AP_SUBNET(255, 255, 255, 0);

// --- Private Helper Methods ---

/**
 * @brief Sets up WiFiManager callbacks and core WiFi settings.
 */
void WiFiHandler::setupWiFiManager() {
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.setAPCallback(configModeCallback);
    
    // Set internal connect timeout to 0 for infinite retries in STA mode
    wifiManager.setConnectTimeout(0); 

    // Initialize Wi-Fi in Station mode and load saved credentials
    WiFi.mode(WIFI_STA);
    WiFi.begin(); 
}

/**
 * @brief Starts the Configuration AP (Access Point) mode indefinitely.
 */
void WiFiHandler::startConfigAP() {
    Serial.println(">>> No saved credentials found. Starting Configuration AP (No Timeout).");

    // Configure the Soft AP IP settings for stability and DNS masking
    WiFi.softAPConfig(AP_LOCAL_IP, AP_GATEWAY, AP_SUBNET);
    
    // Ensure clean reboot after config
    wifiManager.setBreakAfterConfig(true); 
    
    // Start the non-blocking AP portal
    // NOTE: Because there is no saved config, the AP mode should run indefinitely.
    wifiManager.startConfigPortal("airmon_AP", "password"); 
    
    _connectMode = AP_CONFIG_PORTAL;
    // _apModeStartTime is no longer needed since there is no timeout
}

/**
 * @brief Enters the infinite Station (STA) connection mode.
 */
void WiFiHandler::startSTAConnect() {
    Serial.print(">>> Saved config found. Connecting indefinitely to SSID: ");
    Serial.println(WiFi.SSID());

    // The ESP8266 Core handles all reconnection attempts in the background
    _connectMode = STA_CONNECTING;
}

/**
 * @brief Starts the AP with a timeout. Only called when falling back from STA mode.
 * Removed this method as the requirements dictate AP mode only starts when no config is saved,
 * and should then run indefinitely.
 */

// --- Public Methods ---

void WiFiHandler::startConnect() {
    _connectMode = IDLE;

    setupWiFiManager();

    // Decision Logic: AP or STA?
    if (WiFi.SSID().length() == 0) {
        // Case A: No saved config -> Start indefinite AP mode
        startConfigAP();
    } else {
        // Case B: Saved config exists -> Start indefinite STA mode
        startSTAConnect();
    }
}

/**
 * @brief Must be called in the main loop() to check connection status and process AP mode.
 * @return true if Wi-Fi is successfully connected (WL_CONNECTED in WIFI_STA mode), false otherwise.
 */
bool WiFiHandler::handleConnect() {
    if (isStationConnected()) {
        if (_connectMode != IDLE) {
            Serial.println("\n--- SUCCESS! Wi-Fi Connected. ---");
            _connectMode = IDLE; 
        }
        return true;
    }

    return handleConnectionModes();
}

bool WiFiHandler::handleConnectionModes() {
    switch (_connectMode) {
        case STA_CONNECTING:
            // Saved config exists. Waiting for ESP core to connect indefinitely.
            return false;
            
        case AP_CONFIG_PORTAL:
            // CRITICAL: Keep the configuration portal running. No timeout implemented here.
            wifiManager.process(); 
            return false; // Still in AP mode, not connected to the internet.
        
        case IDLE:
            return false;
    }
    
    return false;
}

// --- Status and Utility Methods ---

String WiFiHandler::getWifiStatus() {
    if (isStationConnected()) {
        return "Connected";
    }

    switch (_connectMode) {
        case IDLE:
            return "Idle/Disconnected";
        case STA_CONNECTING:
            return "Connecting...";
        case AP_CONFIG_PORTAL:
            return "AP Config (Persistent)"; 
    }
    
    return "Unknown Status"; 
}

void WiFiHandler::resetSettings() {
    Serial.println("--- Clearing Wi-Fi credentials (Factory Reset). ---");
    wifiManager.resetSettings();
}

// --- Private Static Callback Implementations ---

void WiFiHandler::saveConfigCallback () {
    Serial.println("Wi-Fi credentials saved to storage.");
}

void WiFiHandler::configModeCallback (WiFiManager *myWiFiManager) {
    Serial.println("Entered Configuration Mode.");
    Serial.print("Connect to AP 'airmon_AP' at IP: ");
    Serial.println(WiFi.softAPIP());
}

bool WiFiHandler::isStationConnected() {
    return WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_STA;
}
