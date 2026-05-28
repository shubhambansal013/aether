#line 1 "/home/runner/work/aether/aether/tests/SoftwareSerial.cpp"
#include "SoftwareSerial.h"
#include <algorithm>

SoftwareSerial::SoftwareSerial(int rx, int tx) : read_idx(0) {}
void SoftwareSerial::begin(long baud) {}
int SoftwareSerial::available() { return mock_rx_buffer.size() - read_idx; }
int SoftwareSerial::read() {
    if (read_idx < mock_rx_buffer.size()) return mock_rx_buffer[read_idx++];
    return -1;
}
int SoftwareSerial::peek() {
    if (read_idx < mock_rx_buffer.size()) return mock_rx_buffer[read_idx];
    return -1;
}
size_t SoftwareSerial::readBytes(byte* buffer, size_t length) {
    size_t to_read = std::min(length, (size_t)(mock_rx_buffer.size() - read_idx));
    for (size_t i = 0; i < to_read; i++) {
        buffer[i] = mock_rx_buffer[read_idx++];
    }
    return to_read;
}
