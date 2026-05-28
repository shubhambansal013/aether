#ifndef BLYNKHANDLER_MOCK_H
#define BLYNKHANDLER_MOCK_H

#include "Arduino.h"

class BlynkHandler {
public:
    BlynkHandler();
    void sendData(const char* auth, float p1, float p2, float p10, float t, float h);

    int sendCount = 0;
    struct {
        String auth;
        float p1;
        float p2;
        float p10;
        float t;
        float h;
    } lastSent;
};

#endif
