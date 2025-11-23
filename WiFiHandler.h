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
     * @brief Clears saved Wi-Fi credentials. Used for the power-cycle reset logic.
     */
    void resetSettings();

private:
    WiFiManager wifiManager;

    static void saveConfigCallback();
    static void configModeCallback(WiFiManager *myWiFiManager);
};

#endif
