#include "OLEDDisplay.h"
#include <Wire.h>

#define SSD1306_SETCONTRAST 0x81
#define SCREEN_ADDRESS 0x3C

OLEDDisplay::OLEDDisplay(int sdaPin, int sclPin) 
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET), _sda(sdaPin), _scl(sclPin) {}

void OLEDDisplay::setup() {
    Wire.begin(_sda, _scl);

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { 
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); 
    }
    setBrightness(DEFAULT_OLED_BRIGHTNESS); 
    display.clearDisplay();
    display.display();
}

void OLEDDisplay::update(const SystemData& data) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    drawStatusBar(data);
    drawHeroSection(data.pm2_5);
    drawSecondaryGrid(data);

    display.display();
}

void OLEDDisplay::drawStatusBar(const SystemData& data) {
    drawModeIcon(0, 0, data.isWarmup);
    drawFanIcon(15, 0, (data.isWarmup || !data.isSleeping));
    
    display.setTextSize(1);
    display.setCursor(45, 1);
    if (data.wifiStatus == "AP Config") {
        display.print("SETUP");
    } else {
        if (data.countdown < 10) display.print(" "); 
        display.print(data.countdown);
        display.print("s");
    }

    if (data.showBlynkIcon) {
        display.setCursor(SCREEN_WIDTH - 25, 1);
        display.print("B");
    }

    drawWifiIcon(SCREEN_WIDTH - 12, 1, data.wifiStatus);
    display.drawFastHLine(0, 11, SCREEN_WIDTH, SSD1306_WHITE);
}

void OLEDDisplay::drawHeroSection(float pm2_5) {
    display.setTextSize(1);
    display.setCursor(0, 18);
    display.print("PM 2.5");
    display.setTextSize(3);
    String val = (pm2_5 < 0) ? "---" : String((int)pm2_5);
    int16_t x1, y1; uint16_t w, h;
    display.getTextBounds(val, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(SCREEN_WIDTH - w, 18);
    display.print(val);
}

void OLEDDisplay::drawSecondaryGrid(const SystemData& data) {
    display.drawFastHLine(0, 40, SCREEN_WIDTH, SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 44);
    display.print("P1:"); display.print((int)data.pm1_0);
    display.setCursor(64, 44);
    display.print("P10:"); display.print((int)data.pm10_0);
    display.setCursor(0, 56);
    display.print("T:"); display.print(data.temp, 1); display.print("C");
    display.setCursor(64, 56);
    display.print("H:"); display.print((int)data.hum); display.print("%");
}

void OLEDDisplay::drawWifiIcon(int16_t x, int16_t y, String status) {
    display.fillRect(x, y + 6, 2, 2, SSD1306_WHITE);
    if (status.indexOf("Connected") >= 0) {
        display.fillRect(x + 3, y + 3, 2, 5, SSD1306_WHITE);
        display.fillRect(x + 6, y, 2, 8, SSD1306_WHITE);
    } else if (status == "Connecting...") {
        int stage = (millis() / 400) % 3; 
        if (stage >= 1) display.fillRect(x + 3, y + 3, 2, 5, SSD1306_WHITE);
        if (stage >= 2) display.fillRect(x + 6, y, 2, 8, SSD1306_WHITE);
    } else {
        display.drawLine(x + 4, y + 1, x + 10, y + 7, SSD1306_WHITE);
        display.drawLine(x + 10, y + 1, x + 4, y + 7, SSD1306_WHITE);
    }
}

void OLEDDisplay::drawModeIcon(int16_t x, int16_t y, bool isWarmup) {
    display.drawRect(x, y, 11, 9, SSD1306_WHITE);
    display.setCursor(x + 3, y + 1);
    display.print(isWarmup ? "A" : "P");
}

void OLEDDisplay::drawFanIcon(int16_t x, int16_t y, bool isFanOn) {
    if (!isFanOn) {
        display.drawCircle(x+4, y+4, 3, SSD1306_WHITE);
    } else {
        static float angle = 0;
        angle += 0.6; 
        int8_t x_off = cos(angle) * 3;
        int8_t y_off = sin(angle) * 3;
        display.drawLine(x+4-x_off, y+4-y_off, x+4+x_off, y+4+y_off, SSD1306_WHITE);
        display.drawLine(x+4+y_off, y+4-x_off, x+4-y_off, y+4+x_off, SSD1306_WHITE);
    }
}

void OLEDDisplay::setBrightness(uint8_t val) { setOLEDContrast(val); display.display(); }
void OLEDDisplay::setOLEDContrast(uint8_t val) { Wire.beginTransmission(SCREEN_ADDRESS); Wire.write(0x00); Wire.write(SSD1306_SETCONTRAST); Wire.write(val); Wire.endTransmission(); }
void OLEDDisplay::clear() { display.clearDisplay(); display.display(); }
void OLEDDisplay::printMessage(String l1, String l2) { display.clearDisplay(); display.setTextSize(2); display.setCursor(0,0); display.println(l1); display.setTextSize(1); display.setCursor(0,25); display.println(l2); display.display(); }
