#line 1 "/home/runner/work/aether/aether/tests/SoftwareSerial.h"
#ifndef SOFTWARE_SERIAL_H
#define SOFTWARE_SERIAL_H

class SoftwareSerial {
public:
    SoftwareSerial(int rx, int tx);
    void begin(long baud);
};

#endif
