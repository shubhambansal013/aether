// OLEDDisplay.cpp

#include "OLEDDisplay.h"
#include "pins.h" // Assuming SCREEN_WIDTH, SCREEN_HEIGHT, OLED_RESET, OLED_SDA_PIN, OLED_SCL_PIN are here

// Constructor
OLEDDisplay::OLEDDisplay() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {}

void OLEDDisplay::setup() {
    Serial.println("OLEDDisplay: Initializing Wire library...");
    
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN); 
    Serial.println("OLEDDisplay: Wire library initialized with pins from pins.h.");

    Serial.print("OLEDDisplay: Attempting display.begin(0x3C)...");
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Loop forever if allocation fails
    }
    Serial.println("OLEDDisplay: display.begin() successful.");

    display.clearDisplay();
    
    // Set default text properties
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE); 
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
    
    display.setCursor(0, 35); 
    display.println(line3);
    
    display.display();
}

// ⭐ NEW: Private helper function to draw the Wi-Fi icon
void OLEDDisplay::drawWifiIcon(int16_t x, int16_t y, String status) {
    // Icon dimensions: 12 pixels wide, 8 pixels high
    
    // Draw base arc (outermost bar, always present)
    display.drawPixel(x + 6, y + 8, SSD1306_WHITE); // Center point
    display.drawFastHLine(x + 3, y + 7, 6, SSD1306_WHITE);
    display.drawPixel(x + 2, y + 6, SSD1306_WHITE);
    display.drawPixel(x + 9, y + 6, SSD1306_WHITE);
    
    // Draw status-specific overlay
    if (status == "Connected") {
        // Full signal (draw inner bars)
        display.drawFastHLine(x + 4, y + 5, 4, SSD1306_WHITE); // middle bar
        display.drawPixel(x + 3, y + 4, SSD1306_WHITE);
        display.drawPixel(x + 8, y + 4, SSD1306_WHITE);
        display.drawPixel(x + 5, y + 3, SSD1306_WHITE); // inner bar
        display.drawPixel(x + 6, y + 3, SSD1306_WHITE);
        display.drawPixel(x + 6, y + 2, SSD1306_WHITE); // dot
    } else if (status == "AP Config") {
        // AP Mode / Setup Needed (Draw one bar and a setup indicator)
        display.drawFastHLine(x + 4, y + 5, 4, SSD1306_WHITE);
        display.drawRect(x + 8, y + 0, 4, 4, SSD1306_WHITE); // Box indicator
    } else if (status == "Connecting...") {
        // Connecting (Draw one bar only)
        display.drawFastHLine(x + 4, y + 5, 4, SSD1306_WHITE);
    } else { // Disconnected / Idle
        // Draw an 'X' over the icon
        display.drawLine(x, y, x + 12, y + 8, SSD1306_WHITE);
        display.drawLine(x + 12, y, x, y + 8, SSD1306_WHITE);
    }
}


// ⭐ REVISED: Main display function with new UI layout
void OLEDDisplay::displaySensorDataAndWifiStatus(String wifiStatus, float pm1_0, float pm2_5, float pm10_0, float humidity, float temperature) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    
    // ------------------------------------------------------------------
    // 💡 TOP HEADER ROW (Status & Wi-Fi Icon)
    // ------------------------------------------------------------------
    
    // A. Draw Wi-Fi Icon (top right corner, 12x8)
    drawWifiIcon(SCREEN_WIDTH - 12, 0, wifiStatus);
    
    // B. Display Wi-Fi Status Text (Top Left)
    display.setTextSize(1);
    display.setCursor(0, 0);
    if (wifiStatus == "AP Config") {
         display.print("SETUP AP"); 
    } else {
         // Truncate long status strings to fit beside the icon
         if (wifiStatus.length() > 14) {
             display.print(wifiStatus.substring(0, 11) + "...");
         } else {
             display.print(wifiStatus);
         }
    }


    // ------------------------------------------------------------------
    // 💡 PM2.5 FOCUS (Center Row - Largest Font)
    // ------------------------------------------------------------------
    display.setTextSize(3); // Largest font for main reading
    display.setCursor(0, 14); 
    
    // Left side: Label
    display.print("PM2.5");
    
    // Right side: Value (Right-aligned using screen width)
    String pm2_5_str = String(pm2_5, 0);
    int16_t x1, y1;
    uint16_t w1, h1;
    display.getTextBounds(pm2_5_str, 0, 0, &x1, &y1, &w1, &h1); // Calculate width
    
    // Set cursor to display value aligned to the right edge
    display.setCursor(SCREEN_WIDTH - w1, 14);
    display.println(pm2_5_str);


    // ------------------------------------------------------------------
    // 💡 Secondary PM Values (Mid-Lower Row)
    // ------------------------------------------------------------------
    display.setTextSize(1);
    display.setCursor(0, 40); 
    if (pm2_5 >= 0) { // Check if PM data is valid
        // Left side: PM1.0
        display.print("PM1: ");
        display.print(String(pm1_0, 0));
        
        // Right side: PM10.0 (Starting at the middle of the screen)
        display.setCursor(64, 40); 
        display.print("PM10: ");
        display.println(String(pm10_0, 0));
    } else {
        display.println("PM Sensor Fail");
    }


    // ------------------------------------------------------------------
    // 💡 DHT Data (Bottom Row)
    // ------------------------------------------------------------------
    display.setCursor(0, 54); // Last row
    display.setTextSize(1);
    
    if (!isnan(humidity) && !isnan(temperature)) {
        // Left side: Temperature (T)
        display.print("T: ");
        display.print(String(temperature, 1));
        display.print("C");
        
        // Right side: Humidity (H)
        display.setCursor(64, 54); 
        display.print("H: ");
        display.print(String(humidity, 0));
        display.println("%");
    } else {
        display.println("DHT Sensor Fail");
    }
    
    display.display();
}
