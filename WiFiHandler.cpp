// WiFiHandler.cpp

#include "WiFiHandler.h"

// --- Public Methods ---

bool WiFiHandler::connect() {
    // Set callbacks first
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.setAPCallback(configModeCallback);

    // **************************************************
    // ** AGGRESSIVE WIFI OPTIMIZATIONS **
    // **************************************************

    // Set a longer timeout (10 minutes/600 seconds) for connection attempt/config portal.
    wifiManager.setConnectTimeout(600);

    // Disable captive portal feature for stability during configuration
    // Note: The ESP8266 core usually handles this well, but disabling can help in some cases.
    // wifiManager.setAPClientCheck(false); // Can be removed/tested if issues arise

    // Force the ESP8266 to prioritize 2.4 GHz scanning (often helps stability)
    WiFi.mode(WIFI_STA); // Set to client mode (Station)
    // WiFi.scanDelete(); // Clearing scans can be aggressive; try without it first.

    // If autoConnect fails (e.g., no saved WiFi or saved WiFi unavailable), it opens the AP.
    // The AP name is "airmon_AP", and the password is "password".
    // autoConnect() returns true if connected/false if it times out in the AP.
    return wifiManager.autoConnect("airmon_AP", "password");
}

void WiFiHandler::resetSettings() {
    Serial.println("Clearing Wi-Fi credentials via WiFiHandler.");
    wifiManager.resetSettings();
}

// --- Private Static Callback Implementations ---

void WiFiHandler::saveConfigCallback () {
    // This is called when the user successfully submits new credentials in the config portal.
    Serial.println("Wi-Fi credentials saved to EEPROM.");
}

void WiFiHandler::configModeCallback (WiFiManager *myWiFiManager) {
    // This is called when the ESP enters Configuration AP mode.
    Serial.println("Entered Configuration Mode (Connect to AP airmon_AP).");
    // Optionally print the IP of the configuration portal
    Serial.print("Config Portal IP: ");
    Serial.println(WiFi.softAPIP());
    // 
}