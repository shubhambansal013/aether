#line 1 "/home/runner/work/aether/aether/tests/SoftwareSerial.h"
#ifndef SOFTWARE_SERIAL_H
#define SOFTWARE_SERIAL_H

#include "Arduino.h"
#include <vector>

class SoftwareSerial {
public:
    SoftwareSerial(int rx, int tx);
    void begin(long baud);
    int available();
    int read();
    int peek();
    size_t readBytes(byte* buffer, size_t length);

    // Mock control
    std::vector<byte> mock_rx_buffer;
    int read_idx = 0;
};

#endif
