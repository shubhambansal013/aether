#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>

typedef uint8_t byte;

class String : public std::string {
public:
    String() : std::string("") {}
    String(const char* s) : std::string(s ? s : "") {}
    String(const std::string& s) : std::string(s) {}
    const char* c_str() const { return std::string::c_str(); }
    bool operator==(const char* s) const { return std::string(*this) == std::string(s ? s : ""); }
    bool operator!=(const char* s) const { return !(*this == s); }
};

class WiFiClient {};

#define F(x) x
#define OUTPUT 0x1
#define INPUT 0x0
#define INPUT_PULLUP 0x2
#define LOW 0x0
#define HIGH 0x1
#define WL_CONNECTED 3
#define WL_IDLE_STATUS 0

unsigned long millis();
void delay(unsigned long ms);
void pinMode(int pin, int mode);
void digitalWrite(int pin, int val);

extern std::vector<std::string> serial_logs;

class SerialMock {
public:
    void begin(long baud) {}
    void print(const char* s) {
        if (s == nullptr) return;
        if (!serial_logs.empty() && serial_logs.back().back() != '\n') {
            serial_logs.back() += s;
        } else {
            serial_logs.push_back(s);
        }
    }
    void print(String s) { print(s.c_str()); }
    void println(const char* s) {
        serial_logs.push_back(std::string(s ? s : "") + "\n");
    }
    void println(String s) { println(s.c_str()); }
    void printf(const char* format, ...);
};

extern SerialMock Serial;

extern int wifi_status;

class WiFiMock {
public:
    int status() { return wifi_status; }
    int getMode() { return 1; } // WIFI_STA
};

extern WiFiMock WiFi;

#define WIFI_STA 1

#endif
