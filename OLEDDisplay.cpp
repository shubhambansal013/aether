#include "OLEDDisplay.h"
#include "DHTSensor.h" 
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
    clear();
}

void OLEDDisplay::update(const SystemData& data) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    drawStatusBar(data);
    drawHeroSection(data.pm2_5);
    drawSecondaryGrid(data);

    display.display();
}

// --- Private Specialized Drawing Methods ---

void OLEDDisplay::drawStatusBar(const SystemData& data) {
    drawModeIcon(0, 0, data.isWarmup);
    drawFanIcon(15, 0, (data.isWarmup || !data.isSleeping));
    
    // Center Info (Countdown or Setup)
    display.setTextSize(1);
    display.setCursor(45, 1);
    if (data.wifiStatus == "AP Config") {
        display.print("SETUP");
    } else {
        if (data.countdown < 10) display.print(" "); 
        display.print(data.countdown);
        display.print("s");
    }

    if (data.showBlynkIcon) drawBlynkIndicator();
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
    
    // Row 1: PM 1.0 and PM 10.0
    renderMetric(0, 44, "P1:", data.pm1_0, 0);
    renderMetric(64, 44, "P10:", data.pm10_0, 0);
    
    // Row 2: Temp and Humidity
    renderMetric(0, 56, "T:", data.temp, 1, "C");
    renderMetric(64, 56, "H:", data.hum, 0, "%");
}

/**
 * @brief Generic helper to render a metric with error handling to remove redundant code
 */
void OLEDDisplay::renderMetric(int x, int y, String label, float val, int precision, String unit) {
    display.setCursor(x, y);
    display.print(label);
    if (val <= -1.0 || val == DHTSensor::INVALID_VALUE) {
        display.print("--");
    } else {
        display.print(val, precision);
        display.print(unit);
    }
}

void OLEDDisplay::drawBlynkIndicator() {
    display.setCursor(SCREEN_WIDTH - 25, 1);
    display.print("B");
}

// --- UI Components ---

void OLEDDisplay::drawWifiIcon(int16_t x, int16_t y, String status) {
    display.fillRect(x, y + 6, 2, 2, SSD1306_WHITE); // Base dot
    if (status.indexOf("Connected") >= 0) {
        display.fillRect(x + 3, y + 3, 2, 5, SSD1306_WHITE);
        display.fillRect(x + 6, y, 2, 8, SSD1306_WHITE);
    } else if (status == "Connecting...") {
        int stage = (millis() / 400) % 3; 
        if (stage >= 1) display.fillRect(x + 3, y + 3, 2, 5, SSD1306_WHITE);
        if (stage >= 2) display.fillRect(x + 6, y, 2, 8, SSD1306_WHITE);
    } else {
        // "X" for disconnected
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

// --- Utility Methods ---

void OLEDDisplay::setBrightness(uint8_t val) { 
    Wire.beginTransmission(SCREEN_ADDRESS); 
    Wire.write(0x00); 
    Wire.write(SSD1306_SETCONTRAST); 
    Wire.write(val); 
    Wire.endTransmission(); 
    display.display(); 
}

void OLEDDisplay::clear() { display.clearDisplay(); display.display(); }

void OLEDDisplay::printMessage(String l1, String l2) { 
    display.clearDisplay(); 
    display.setTextSize(2); 
    display.setCursor(0,0); 
    display.println(l1); 
    display.setTextSize(1); 
    display.setCursor(0,25); 
    display.println(l2); 
    display.display(); 
}
