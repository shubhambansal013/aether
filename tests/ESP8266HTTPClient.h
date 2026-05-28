#ifndef ESP8266HTTPCLIENT_H
#define ESP8266HTTPCLIENT_H
#include "Arduino.h"
#define HTTP_CODE_OK 200
class HTTPClient {
public:
    bool begin(class WiFiClient& client, String url) { return true; }
    int GET() { return 0; }
    String getString() { return ""; }
    void end() {}
    String errorToString(int code) { return ""; }
};
#endif
