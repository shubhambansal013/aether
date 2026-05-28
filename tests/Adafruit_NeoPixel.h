#ifndef ADAFRUIT_NEOPIXEL_H
#define ADAFRUIT_NEOPIXEL_H

#define NEO_GRB 0
#define NEO_KHZ800 0

class Adafruit_NeoPixel {
public:
    Adafruit_NeoPixel(uint16_t n, uint16_t p, uint8_t t) {}
    void begin() {}
    void show() {}
    void setPixelColor(uint16_t n, uint32_t c) {}
    void setBrightness(uint8_t b) {}
};

#endif
