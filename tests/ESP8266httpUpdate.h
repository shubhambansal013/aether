#ifndef ESP8266HTTPUPDATE_H
#define ESP8266HTTPUPDATE_H
#include "Arduino.h"

enum t_httpUpdate_return {
    HTTP_UPDATE_FAILED,
    HTTP_UPDATE_NO_UPDATES,
    HTTP_UPDATE_OK
};

extern t_httpUpdate_return mock_update_return;

class ESP8266HTTPUpdate {
public:
    t_httpUpdate_return update(class WiFiClient& client, const char* url) { return mock_update_return; }
    int getLastError() { return 0; }
    String getLastErrorString() { return ""; }
};

extern ESP8266HTTPUpdate ESPhttpUpdate;

#endif
