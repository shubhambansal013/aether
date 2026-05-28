#line 1 "/home/runner/work/aether/aether/tests/WiFiClientSecureBearSSL.h"
#ifndef WIFICLIENTSECUREBEARSSL_H
#define WIFICLIENTSECUREBEARSSL_H
#include "Arduino.h"
namespace BearSSL {
class WiFiClientSecure : public WiFiClient {
public:
    void setInsecure() {}
};
}
#endif
