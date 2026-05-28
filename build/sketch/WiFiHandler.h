#line 1 "/home/runner/work/aether/aether/WiFiHandler.h"
// WiFiHandler.h

#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include <Arduino.h>
#include <WiFiManager.h>

class WiFiHandler {
public:
    enum WiFiConnectionMode {
        IDLE,                // No connection attempt active, or connected
        STA_CONNECTING,      // Attempting to connect to a saved AP (indefinitely)
        AP_CONFIG_PORTAL     // Running configuration portal as an AP (indefinitely)
    };

    /**
     * @brief Starts the Wi-Fi connection process in a non-blocking manner.
     * * If saved configuration exists, it attempts to connect indefinitely (STA_CONNECTING).
     * If no configuration exists, it starts the Configuration AP indefinitely (AP_CONFIG_PORTAL).
     */
    void startConnect(); // Simplified signature, no timeouts needed

    /**
     * @brief Handles the ongoing Wi-Fi connection attempt. Should be called repeatedly in loop().
     * @return true if connected successfully, false if still connecting or failed.
     */
    bool handleConnect();

    /**
     * @brief Returns a string representation of the current WiFi status.
     * @return A string indicating the WiFi status.
     */
    String getWifiStatus();

    /**
     * @brief Clears saved Wi-Fi credentials.
     */
    void resetSettings();

private:
    WiFiManager wifiManager;
    WiFiConnectionMode _connectMode = IDLE;

    // --- Private Helper Methods (Match the new .cpp structure) ---
    void setupWiFiManager();
    void startConfigAP();
    void startSTAConnect();
    bool isStationConnected();
    bool handleConnectionModes();
    // Removed processAPTimeout() as AP mode is now indefinite when no config is present.

    // --- Static Callbacks ---
    static void saveConfigCallback();
    static void configModeCallback(WiFiManager *myWiFiManager);
};

#endif // WIFI_HANDLER_H
