#include "OLEDDisplay.h"

OLEDDisplay::OLEDDisplay() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {}

void OLEDDisplay::setup() {
    Wire.begin();
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Don't proceed, loop forever
    }
    display.display();
    delay(2000);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("OLED Initialized!");
    display.display();
    Serial.println("OLED Display initialized.");
}

void OLEDDisplay::clear() {
    display.clearDisplay();
}

void OLEDDisplay::printMessage(String line1, String line2) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println(line1);
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println(line2);
    display.display();
}

void OLEDDisplay::printMessage(String line1, String line2, String line3) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println(line1);
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println(line2);
    display.setCursor(0, 30);
    display.println(line3);
    display.display();
}