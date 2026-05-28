#ifndef ESP8266HTTPCLIENT_H
#define ESP8266HTTPCLIENT_H
#include "Arduino.h"
#define HTTP_CODE_OK 200

extern int mock_http_code;
extern String mock_payload;

class HTTPClient {
public:
    bool begin(class WiFiClient& client, String url) { return true; }
    int GET() { return mock_http_code; }
    String getString() { return mock_payload; }
    void end() {}
    String errorToString(int code) { return "error"; }
};
#endif
