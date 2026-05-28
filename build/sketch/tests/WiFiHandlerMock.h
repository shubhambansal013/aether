#line 1 "/home/runner/work/aether/aether/tests/WiFiHandlerMock.h"
#ifndef WIFIHANDLER_MOCK_H
#define WIFIHANDLER_MOCK_H

#include "Arduino.h"

class WiFiHandler {
public:
    void startConnect();
    bool handleConnect();
    String getWifiStatus();
    void resetSettings();

    String mock_status = "Connected";
};

#endif
