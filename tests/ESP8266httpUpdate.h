#ifndef ESP8266HTTPUPDATE_H
#define ESP8266HTTPUPDATE_H
#include "Arduino.h"
#include "ESP8266HTTPClient.h"

enum t_httpUpdate_return { HTTP_UPDATE_FAILED, HTTP_UPDATE_NO_UPDATES, HTTP_UPDATE_OK };
class ESP8266HTTPUpdate {
public:
    t_httpUpdate_return update(class WiFiClient& client, const char* url) { return HTTP_UPDATE_NO_UPDATES; }
    t_httpUpdate_return update(const String& url, const String& currentVersion = "") { return HTTP_UPDATE_NO_UPDATES; }
    void setFollowRedirects(followRedirects_t follow) {}
    void setInsecure() {}
    int getLastError() { return 0; }
    String getLastErrorString() { return ""; }
};
extern ESP8266HTTPUpdate ESPhttpUpdate;
#endif
