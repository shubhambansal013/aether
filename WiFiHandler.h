// WiFiHandler.h

#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include <Arduino.h>
#include <WiFiManager.h>

class WiFiHandler {
public:
    /**
     * @brief Initiates Wi-Fi connection with a two-step process: quick connect, then fallback to AP.
     * @param quickConnectTimeoutMs Time for initial connection attempt (e.g., 20000ms).
     * @param apTimeoutSec Time for configuration portal to stay active (e.g., 600s).
     * @return true if connected successfully, false if timeout occurs in AP mode.
     */
    bool connect(unsigned long quickConnectTimeoutMs, int apTimeoutSec);

    /**
     * @brief Starts the Wi-Fi connection attempt in a non-blocking manner.
     * @param quickConnectTimeoutMs Time for initial connection attempt.
     * @param apTimeoutSec Time for configuration portal to stay active.
     */
    void startConnect(unsigned long quickConnectTimeoutMs, int apTimeoutSec);

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
    int _apTimeoutSec;
    unsigned long _connectStartTime;
    bool _connectionAttemptInProgress;
    bool _apModeStarted; // New flag to track if AP mode has been initiated

    static void saveConfigCallback();
    static void configModeCallback(WiFiManager *myWiFiManager);
};

#endif
