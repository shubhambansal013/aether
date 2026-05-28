#ifndef WIFICLIENTSECUREBEARSSL_H
#define WIFICLIENTSECUREBEARSSL_H
#include "Arduino.h"
namespace BearSSL {
class WiFiClientSecure : public WiFiClient {
public:
    void setInsecure() {}
    void setBufferSizes(uint16_t rx, uint16_t tx) {}
};
}
#endif
