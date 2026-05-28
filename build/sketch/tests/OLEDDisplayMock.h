#line 1 "/home/runner/work/aether/aether/tests/OLEDDisplayMock.h"
#ifndef OLEDDISPLAY_MOCK_H
#define OLEDDISPLAY_MOCK_H

#include "Arduino.h"
#include "SystemData.h"

class OLEDDisplay {
public:
    OLEDDisplay(int sda, int scl);
    void setup();
    void update(const SystemData& d);
    void clear();
    void printMessage(String l1, String l2);

    int updateCount = 0;
    int clearCount = 0;
    SystemData lastData;
};

#endif
