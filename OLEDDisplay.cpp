#include "OLEDDisplay.h"

OLEDDisplay::OLEDDisplay() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {}

void OLEDDisplay::setup() {
    Serial.println("OLEDDisplay: Initializing Wire library...");
    Wire.begin();
    Serial.println("OLEDDisplay: Wire library initialized.");

    Serial.print("OLEDDisplay: Attempting display.begin(0x3C)...");
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Don't proceed, loop forever
    }
    Serial.println("OLEDDisplay: display.begin() successful.");

    // Set contrast to maximum (0-255). This often helps with blank displays.
    Serial.println("OLEDDisplay: Setting display contrast to max...");
    display.setContrast(255);

    Serial.println("OLEDDisplay: Calling display.display() first time...");
    display.display();
    Serial.println("OLEDDisplay: First display.display() called. Delaying 2s.");
    delay(2000);

    Serial.println("OLEDDisplay: Clearing display...");
    display.clearDisplay();
    Serial.println("OLEDDisplay: Setting text properties...");
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    Serial.println("OLEDDisplay: Printing 'OLED Initialized!' message...");
    display.println("OLED Initialized!");
    Serial.println("OLEDDisplay: Calling display.display() second time...");
    display.display();
    Serial.println("OLED Display initialized and message displayed.");
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