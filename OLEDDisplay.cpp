#include "OLEDDisplay.h"
#include "pins.h"

// Constructor
OLEDDisplay::OLEDDisplay() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {}

void OLEDDisplay::setup() {
    Serial.println("OLEDDisplay: Initializing Wire library...");
    
    // FIX: Explicitly set SDA and SCL from pins.h
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN); 
    Serial.println("OLEDDisplay: Wire library initialized with pins from pins.h.");

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

void OLEDDisplay::displaySensorDataAndWifiStatus(String wifiStatus, float pm1_0, float pm2_5, float pm10_0, float humidity, float temperature) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    
    // Display WiFi Status at the top (smaller font)
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("WiFi: " + wifiStatus);

    // PM2.5 (most important, larger font)
    display.setTextSize(2);
    display.setCursor(0, 15); 
    display.print("PM2.5: ");
    display.println(String(pm2_5, 0));
    
    // PM1.0 and PM10.0 (smaller font, on one line if possible)
    display.setTextSize(1);
    display.setCursor(0, 35); 
    display.print("PM1.0:");
    display.print(String(pm1_0, 0));
    display.print(" PM10:");
    display.println(String(pm10_0, 0));

    // Humidity and Temperature (smaller font, on one line if possible)
    display.setCursor(0, 50); 
    if (!isnan(humidity) && !isnan(temperature)) {
        display.print("H: ");
        display.print(String(humidity, 0));
        display.print("% T: ");
        display.print(String(temperature, 1));
        display.println("C");
    } else {
        display.println("DHT Fail");
    }
    
    display.display();
}
