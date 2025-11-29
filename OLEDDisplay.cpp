#include "OLEDDisplay.h"

// Constructor
OLEDDisplay::OLEDDisplay() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {}

void OLEDDisplay::setup() {
    Serial.println("OLEDDisplay: Initializing Wire library...");
    
    // FIX: Explicitly set SDA to D2 and SCL to D1
    Wire.begin(D2, D1); 
    Serial.println("OLEDDisplay: Wire library initialized with D2(SDA) and D1(SCL).");

    Serial.print("OLEDDisplay: Attempting display.begin(0x3C)...");
    // SSD1306_SWITCHCAPVCC generates the high voltage from the 3.3v line internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Loop forever if allocation fails
    }
    Serial.println("OLEDDisplay: display.begin() successful.");

    // FIX: Removed setContrast(255) as it is not supported by the standard Adafruit_SSD1306 library.
    // The library handles contrast initialization automatically in begin().

    display.clearDisplay();
    
    // Set default text properties
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE); // Use SSD1306_WHITE for compatibility
    display.setCursor(0,0);
    
    display.println("OLED Initialized!");
    display.display();
    
    Serial.println("OLED Display initialized and message displayed.");
}

void OLEDDisplay::clear() {
    display.clearDisplay();
    display.display();
}

void OLEDDisplay::printMessage(String line1, String line2) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    
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
    display.setTextColor(SSD1306_WHITE);
    
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println(line1);
    
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println(line2);
    
    display.setCursor(0, 35); // Adjusted spacing slightly
    display.println(line3);
    
    display.display();
}

void OLEDDisplay::displaySensorDataAndWifiStatus(String wifiStatus, String line1, String line2, String line3) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    
    // Display WiFi Status at the top (smaller font)
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("WiFi: " + wifiStatus);

    // Display sensor data below
    display.setTextSize(2);
    display.setCursor(0, 15); // Adjust Y-coordinate to avoid overlap
    display.println(line1);
    
    display.setTextSize(1);
    display.setCursor(0, 35); // Adjust Y-coordinate
    display.println(line2);
    
    display.setCursor(0, 50); // Adjust Y-coordinate
    display.println(line3);
    
    display.display();
}
