#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>
#include <string.h>
#include <iostream>
#include <string>

typedef uint8_t byte;
typedef std::string String;

class WiFiClient {};

#define F(x) x
#define OUTPUT 0x1
#define INPUT 0x0
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
    void print(String s) { std::cout << s; }
    void println(const char* s) { std::cout << s << std::endl; }
    void println(String s) { std::cout << s << std::endl; }
    void printf(const char* format, ...) {}
};

extern SerialMock Serial;

class WiFiMock {
public:
    int status() { return WL_CONNECTED; }
};

extern WiFiMock WiFi;

#endif
