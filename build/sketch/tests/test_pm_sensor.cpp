#line 1 "/home/runner/work/aether/aether/tests/test_pm_sensor.cpp"
#include "Arduino.h"
#include <vector>
#include <assert.h>
#include <iostream>
#include "SoftwareSerial.h"

unsigned long mock_millis = 0;
unsigned long millis() { return mock_millis; }
void delay(unsigned long ms) { mock_millis += ms; }
void pinMode(int pin, int mode) {}
void digitalWrite(int pin, int val) {}

SerialMock Serial;

#define private public
#include "../PMSensor.h"
#undef private

void test_pm_parsing_valid() {
    std::cout << "Running test_pm_parsing_valid..." << std::endl;

    PMSensor pm(14, 12);

    // Construct a valid 32-byte PMS packet
    std::vector<byte> packet(32, 0);
    packet[0] = 0x42;
    packet[1] = 0x4D;

    // PM values are at 10-11, 12-13, 14-15
    // PM1.0 = 10 (0x000A)
    packet[10] = 0x00; packet[11] = 0x0A;
    // PM2.5 = 20 (0x0014)
    packet[12] = 0x00; packet[13] = 0x14;
    // PM10 = 30 (0x001E)
    packet[14] = 0x00; packet[15] = 0x1E;

    // Checksum: sum of bytes 0..29
    uint16_t sum = 0;
    for(int i=0; i<30; i++) sum += packet[i];
    packet[30] = (sum >> 8) & 0xFF;
    packet[31] = sum & 0xFF;

    pm._pmSerial.mock_rx_buffer = packet;

    float pm1, pm2, pm10;
    bool success = pm.readData(pm1, pm2, pm10);

    assert(success == true);
    assert(pm1 == 10.0);
    assert(pm2 == 20.0);
    assert(pm10 == 30.0);
    std::cout << "test_pm_parsing_valid passed!" << std::endl;
}

void test_pm_parsing_invalid_checksum() {
    std::cout << "Running test_pm_parsing_invalid_checksum..." << std::endl;

    PMSensor pm(14, 12);

    std::vector<byte> packet(32, 0);
    packet[0] = 0x42;
    packet[1] = 0x4D;
    packet[30] = 0xFF; // Wrong checksum
    packet[31] = 0xFF;

    pm._pmSerial.mock_rx_buffer = packet;

    float pm1, pm2, pm10;
    bool success = pm.readData(pm1, pm2, pm10);

    assert(success == false);
    std::cout << "test_pm_parsing_invalid_checksum passed!" << std::endl;
}

void test_pm_parsing_header_sync() {
    std::cout << "Running test_pm_parsing_header_sync..." << std::endl;

    PMSensor pm(14, 12);

    // Some garbage before the actual packet
    std::vector<byte> data = {0x00, 0x42, 0x01, 0x42, 0x4D};
    // Valid packet content
    for(int i=0; i<27; i++) data.push_back(0); // Rest of the packet (we need 32 total, we have 2 header + 27 = 29. Wait.
    // 0x42, 0x4D are at index 3, 4.
    // Full packet starts at index 3.
    // Bytes 3..34 would be the packet.
    while(data.size() < 35) data.push_back(0);

    // Fix checksum for packet starting at index 3
    uint16_t sum = 0;
    for(int i=3; i<3+30; i++) sum += data[i];
    data[3+30] = (sum >> 8) & 0xFF;
    data[3+31] = sum & 0xFF;

    pm._pmSerial.mock_rx_buffer = data;

    float pm1, pm2, pm10;
    bool success = pm.readData(pm1, pm2, pm10);

    assert(success == true);
    std::cout << "test_pm_parsing_header_sync passed!" << std::endl;
}

int main() {
    test_pm_parsing_valid();
    test_pm_parsing_invalid_checksum();
    test_pm_parsing_header_sync();
    std::cout << "All PMSensor tests passed!" << std::endl;
    return 0;
}
