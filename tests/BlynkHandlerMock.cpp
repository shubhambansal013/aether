#include "BlynkHandlerMock.h"

BlynkHandler::BlynkHandler() {}
void BlynkHandler::sendData(const char* a, float p1, float p2, float p10, float t, float h) {
    sendCount++;
    lastSent.auth = a;
    lastSent.p1 = p1;
    lastSent.p2 = p2;
    lastSent.p10 = p10;
    lastSent.t = t;
    lastSent.h = h;
}
