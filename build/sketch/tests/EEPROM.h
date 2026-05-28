#line 1 "/home/runner/work/aether/aether/tests/EEPROM.h"
#ifndef EEPROM_H
#define EEPROM_H

#include <Arduino.h>
#include <map>
#include <vector>

class EEPROMMock {
public:
    void begin(int size) {}
    void commit() {}

    template<typename T>
    void get(int address, T& data) {
        if (storage.find(address) != storage.end()) {
            data = *(T*)&storage[address][0];
        } else {
            memset(&data, 0xFF, sizeof(T));
        }
    }

    template<typename T>
    void put(int address, const T& data) {
        storage[address].resize(sizeof(T));
        memcpy(&storage[address][0], &data, sizeof(T));
    }

private:
    std::map<int, std::vector<byte>> storage;
};

extern EEPROMMock EEPROM;

#endif
