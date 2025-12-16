#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include "WiFiHandler.h"
#include "ResetHandler.h"
#include "PMSensor.h"
#include "BlynkHandler.h"
#include "blynk_config.h"
#include "pins.h"
#include "OTAHandler.h"
#include "DHTSensor.h"
#include "OLEDDisplay.h"
#include "RGBLEDHandler.h"

// ----------------------------------------------------------------------
// ⚙️ FIRMWARE VERSION & SYSTEM CONSTANTS
// ----------------------------------------------------------------------
const char* FIRMWARE_VERSION = "V1.1.0 - WS2812 Update";
const long DEBUG_BAUD_RATE = 115200;
const int SETUP_DELAY_MS = 100;

// --- Sensor Constants ---
const long SENSOR_BAUD_RATE = 9600;
const unsigned long BLYNK_SEND_INTERVAL_MS = 60000L;
const bool USE_MOCK_DATA = false;

// --- Loop delay ---
const long LOOP_DELAY = 1000;

// --- OTA Update Constants ---
const char* GITHUB_REPO_USER = "shubhambansal013";
const char* GITHUB_REPO_NAME = "pawan";
const char* FIRMWARE_BIN_NAME = "firmware.bin";

// --- Global Variables/Instances ---
WiFiHandler wifiHandler;
ResetHandler resetHandler(wifiHandler);
PMSensor pmSensor(PM_SENSOR_RX_PIN, PM_SENSOR_TX_PIN);
BlynkHandler blynkHandler;
OTAHandler otaHandler(FIRMWARE_VERSION, GITHUB_REPO_USER, GITHUB_REPO_NAME, FIRMWARE_BIN_NAME);
DHTSensor dhtSensor;
OLEDDisplay oledDisplay;

// UPDATED: Now uses single WS2812_DATA_PIN instead of R, G, B pins
RGBLEDHandler rgbLEDHandler(WS2812_DATA_PIN);

// Cloud Variables
float pm1_0_val;
float pm2_5_val;
float pm10_0_val;

unsigned long lastSendTime = 0;
bool _otaInitialized = false; 

// ----------------------------------------------------------------------
// SETUP & LOOP
// ----------------------------------------------------------------------

void setup() {
    Serial.begin(DEBUG_BAUD_RATE);
    delay(SETUP_DELAY_MS);
    Serial.print("\nFirmware Version: ");
    Serial.println(FIRMWARE_VERSION);

    // 1. Initialize OLED Display FIRST
    oledDisplay.setup();
    oledDisplay.printMessage("System", "Booting...");

    // 2. Initialize WS2812 LED
    rgbLEDHandler.setup();
    rgbLEDHandler.startupSequence(); // Confirms R->G->B connectivity

    // 3. Check for Power Cycle Reset
    resetHandler.checkPowerCycles();

    // 4. Initialize Wi-Fi
    oledDisplay.printMessage("WiFi", "Starting...");
    wifiHandler.startConnect();

    // 5. Initialize Sensors
    pmSensor.begin(SENSOR_BAUD_RATE);
    dhtSensor.setup();
}

void loop() {
    // 1. Handle Wi-Fi Connection State
    wifiHandler.handleConnect();
    bool currentlyConnected = (WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_STA);

    // 2. Read Sensor Data
    bool sensorDataAvailable = pmSensor.readData(pm1_0_val, pm2_5_val, pm10_0_val, USE_MOCK_DATA);
    float h = dhtSensor.readHumidity();
    float t = dhtSensor.readTemperature();

    // 3. Update LED (Simplified immediate update)
    rgbLEDHandler.updateLED(pm2_5_val, sensorDataAvailable);

    // 4. Update OLED Display
    String wifiStatusStr = wifiHandler.getWifiStatus();
    oledDisplay.displaySensorDataAndWifiStatus(wifiStatusStr, pm1_0_val, pm2_5_val, pm10_0_val, h, t);

    // 5. Blynk Data Transmission
    if (millis() - lastSendTime > BLYNK_SEND_INTERVAL_MS) {
        if (sensorDataAvailable && currentlyConnected) {
            blynkHandler.sendData(BLYNK_AUTH_TOKEN, pm1_0_val, pm2_5_val, pm10_0_val, t, h);
            lastSendTime = millis();
        }
    }
    
    // 6. Serial Debug Output
    if (sensorDataAvailable) {
        Serial.print("PM2.5: "); Serial.println(pm2_5_val);
    }
        
    delay(LOOP_DELAY);
}
