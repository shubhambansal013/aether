#ifndef ESP8266HTTPCLIENT_H
#define ESP8266HTTPCLIENT_H
#include "Arduino.h"
#define HTTP_CODE_OK 200
enum followRedirects_t {
    HTTPC_DISABLE_FOLLOW_REDIRECTS,
    HTTPC_STRICT_FOLLOW_REDIRECTS,
    HTTPC_FORCE_FOLLOW_REDIRECTS
};
class HTTPClient {
public:
    bool begin(class WiFiClient& client, String url) { return true; }
    void setFollowRedirects(followRedirects_t follow) {}
    int GET() { return 0; }
    String getString() { return ""; }
    void end() {}
    String errorToString(int code) { return ""; }
};
#endif
