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

// ⭐ REVISED: Private helper function to draw a clear, 3-bar vertical Wi-Fi icon
void OLEDDisplay::drawWifiIcon(int16_t x, int16_t y, String status) {
    // Icon size: 8 pixels wide, 8 pixels high
    int barWidth = 2;
    int spacing = 1;
    
    // Bar 1 (Smallest - lowest signal)
    display.drawRect(x, y + 5, barWidth, 3, SSD1306_WHITE);

    if (status == "Connected" || status == "AP Config" || status == "Connecting...") {
        // Bar 2 (Medium)
        display.drawRect(x + barWidth + spacing, y + 3, barWidth, 5, SSD1306_WHITE);
        
        if (status == "Connected") {
            // Bar 3 (Largest - full signal)
            display.drawRect(x + 2 * (barWidth + spacing), y + 0, barWidth, 8, SSD1306_WHITE);
        }
    }
    
    if (status == "AP Config") {
        // Draw a small dot or asterisk below the icon to denote AP mode
        display.drawPixel(x + 4, y + 10, SSD1306_WHITE);
    }
    
    if (status == "Idle/Disconnected" || status == "Unknown") {
        // Clear all but bar 1 and draw an X over the whole area
        display.fillRect(x + barWidth + spacing, y, 2 * (barWidth + spacing) + barWidth, 8, SSD1306_BLACK);
        display.drawLine(x, y, x + 8, y + 8, SSD1306_WHITE);
        display.drawLine(x + 8, y, x, y + 8, SSD1306_WHITE);
    }
}


// ⭐ REVISED: Main display function with improved layout and font sizes
void OLEDDisplay::displaySensorDataAndWifiStatus(String wifiStatus, float pm1_0, float pm2_5, float pm10_0, float humidity, float temperature) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    
    // ------------------------------------------------------------------
    // 💡 TOP HEADER ROW (Status & Wi-Fi Icon)
    // ------------------------------------------------------------------
    
    // A. Draw Wi-Fi Icon (Top Right Corner)
    // Icon is 8x8, placing it at X=120, Y=0 (128 - 8)
    drawWifiIcon(SCREEN_WIDTH - 8, 0, wifiStatus);
    
    // B. Display Wi-Fi Status Text (Top Left)
    display.setTextSize(1);
    display.setCursor(0, 0);
    
    // Truncate/rename status to fit beside the icon (128 - 10 pixels for icon/padding)
    String status_display;
    if (wifiStatus == "AP Config") {
         status_display = "SETUP AP"; 
    } else if (wifiStatus == "Connecting...") {
         status_display = "Connecting...";
    } else if (wifiStatus == "Idle/Disconnected") {
         status_display = "DISCONNECTED";
    } else {
         status_display = wifiStatus;
    }
    display.print(status_display);


    // ------------------------------------------------------------------
    // 💡 PM2.5 FOCUS (Large, Clear Font)
    // ------------------------------------------------------------------
    
    // ⚠️ FIX: Reduced font size to 2 to prevent merging with surrounding text
    display.setTextSize(2); 
    display.setCursor(0, 15); 
    
    // Left side: Label
    display.print("PM2.5");
    
    // Right side: Value
    String pm2_5_str = String(pm2_5, 0);
    int16_t x1, y1;
    uint16_t w1, h1;
    display.getTextBounds(pm2_5_str, 0, 0, &x1, &y1, &w1, &h1); // Calculate width
    
    // Place value right-aligned at X=128
    display.setCursor(SCREEN_WIDTH - w1, 15);
    display.println(pm2_5_str);

    // Draw a dividing line
    display.drawFastHLine(0, 36, SCREEN_WIDTH, SSD1306_WHITE);


    // ------------------------------------------------------------------
    // 💡 Secondary PM and DHT Data (Bottom Half - Two Rows)
    // ------------------------------------------------------------------
    display.setTextSize(1);
    
    // ROW 1: PM1.0 & PM10.0 (Starting at Y=40)
    display.setCursor(0, 40); 
    if (pm2_5 >= 0) {
        // Left side: PM1.0
        display.print("PM1:");
        display.print(String(pm1_0, 0));
        
        // Right side: PM10.0 (Starting at the middle of the screen X=64)
        display.setCursor(64, 40); 
        display.print("PM10:");
        display.println(String(pm10_0, 0));
    } else {
        display.println("PM Sensor Fail");
    }

    // ROW 2: DHT Data (Starting at Y=54)
    display.setCursor(0, 54); 
    if (!isnan(humidity) && !isnan(temperature)) {
        // Left side: Temperature (T)
        display.print("T:");
        display.print(String(temperature, 1));
        display.print("C");
        
        // Right side: Humidity (H)
        display.setCursor(64, 54); 
        display.print("H:");
        display.print(String(humidity, 0));
        display.println("%");
    } else {
        display.println("DHT Sensor Fail");
    }
    
    display.display();
}
