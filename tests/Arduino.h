#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>
#include <string.h>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <stdio.h>

typedef uint8_t byte;
class String : public std::string {
public:
    String() : std::string() {}
    String(const char* s) : std::string(s) {}
    String(const std::string& s) : std::string(s) {}
    void trim() {}
    const char* c_str() const { return std::string::c_str(); }
    size_t length() const { return std::string::length(); }
};

class WiFiClient {};

#define F(x) x
#define OUTPUT 0x1
#define INPUT 0x0
#define INPUT_PULLUP 0x2
#define LOW 0x0
#define HIGH 0x1
#define WL_CONNECTED 3

unsigned long millis();
void delay(unsigned long ms);
void pinMode(int pin, int mode);
void digitalWrite(int pin, int val);

class SerialMock {
public:
    void begin(long baud) {}
    void print(const char* s) { std::cout << s; }
    void print(String s) { std::cout << (std::string)s; }
    void print(uint32_t n) { std::cout << n; }
    void println(const char* s) { std::cout << s << std::endl; }
    void println(String s) { std::cout << (std::string)s << std::endl; }
    void println(uint32_t n) { std::cout << n << std::endl; }
    void printf(const char* format, ...) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        std::cout << std::endl;
    }
};

extern SerialMock Serial;

class WiFiMock {
public:
    int status() { return WL_CONNECTED; }
};

extern WiFiMock WiFi;

class ESPMock {
public:
    uint32_t getFreeHeap() { return 40000; }
};
extern ESPMock ESP;

#endif
