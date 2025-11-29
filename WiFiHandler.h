// WiFiHandler.h

#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include <Arduino.h>
#include <WiFiManager.h>

class WiFiHandler {
public:
    enum WiFiConnectionMode {
        IDLE,            // No connection attempt active, or connected
        STA_CONNECTING,  // Attempting to connect to a saved AP
        AP_CONFIG_PORTAL // Running configuration portal as an AP
    };

    /**
     * @brief Starts the Wi-Fi connection attempt in a non-blocking manner.
     * @param quickConnectTimeoutMs Time for initial STA connection attempt.
     * @param apModeTimeoutSec Time for configuration portal to stay active.
     */
    void startConnect(unsigned long quickConnectTimeoutMs, unsigned long apModeTimeoutSec);

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
     * @brief Clears saved Wi-Fi credentials. Used for the power-cycle reset logic.
     */
    void resetSettings();

private:
    WiFiManager wifiManager;
    unsigned long _quickConnectTimeoutMs;
    unsigned long _apModeTimeoutSec; // Renamed for clarity
    unsigned long _connectStartTime;
    WiFiConnectionMode _connectMode = IDLE;
    unsigned long _apModeStartTime; // To track duration of config portal

    static void saveConfigCallback();
    static void configModeCallback(WiFiManager *myWiFiManager);
};
#endif
