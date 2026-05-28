#ifndef ESP8266HTTPUPDATE_H
#define ESP8266HTTPUPDATE_H
#include "Arduino.h"
enum t_httpUpdate_return { HTTP_UPDATE_FAILED, HTTP_UPDATE_NO_UPDATES, HTTP_UPDATE_OK };
class ESP8266HTTPUpdate {
public:
    t_httpUpdate_return update(class WiFiClient& client, const char* url) { return HTTP_UPDATE_NO_UPDATES; }
    int getLastError() { return 0; }
    String getLastErrorString() { return ""; }
};
extern ESP8266HTTPUpdate ESPhttpUpdate;
#endif
