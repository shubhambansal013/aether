#line 1 "/home/runner/work/aether/aether/tests/WiFiHandlerMock.cpp"
#include "WiFiHandlerMock.h"

void WiFiHandler::startConnect() {}
bool WiFiHandler::handleConnect() { return true; }
String WiFiHandler::getWifiStatus() { return mock_status; }
void WiFiHandler::resetSettings() {}
