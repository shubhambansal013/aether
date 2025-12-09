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
const char* FIRMWARE_VERSION = "V1.0.10 - OLED Fix";
const long DEBUG_BAUD_RATE = 115200;
const int SETUP_DELAY_MS = 100;

// --- Sensor Constants ---
const long SENSOR_BAUD_RATE = 9600;
const unsigned long BLYNK_SEND_INTERVAL_MS = 60000L;
const bool USE_MOCK_DATA = false;

// --- Standby Mode Timing ---
const unsigned long STANDBY_MODE_DELAY_MS = 5000L; // X = 5 seconds
bool standbyModeSet = false;
unsigned long firstRunTime = 0; // To store the time setup finishes

// --- Loop delay ---
const long LOOP_DELAY = 1000;
// ----------------------------------------------------------------------

// --- OTA Update Constants (Unchanged) ---
const char* GITHUB_REPO_USER = "shubhambansal013";
const char* GITHUB_REPO_NAME = "pawan";
const char* FIRMWARE_BIN_NAME = "firmware.bin";
// ----------------------------------------------------------------------

// --- Global Variables/Instances ---
WiFiHandler wifiHandler;
ResetHandler resetHandler(wifiHandler);
PMSensor pmSensor(PM_SENSOR_RX_PIN, PM_SENSOR_TX_PIN);
BlynkHandler blynkHandler;
OTAHandler otaHandler(FIRMWARE_VERSION, GITHUB_REPO_USER, GITHUB_REPO_NAME, FIRMWARE_BIN_NAME);
DHTSensor dhtSensor;
OLEDDisplay oledDisplay;
RGBLEDHandler rgbLEDHandler(RGB_LED_RED_PIN, RGB_LED_GREEN_PIN, RGB_LED_BLUE_PIN);

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

    // 1. Initialize OLED Display FIRST to show status during boot
    oledDisplay.setup();
    oledDisplay.printMessage("System", "Booting...");

    // Initialize RGB LED
    rgbLEDHandler.setup();
    // 🧹 NEW: Run the startup sequence to verify colors
    rgbLEDHandler.startupSequence();

    // 2. Check for Power Cycle Reset (Must be run before Wi-Fi)
    resetHandler.checkPowerCycles();

    // 3. Initialize Wi-Fi (STARTS NON-BLOCKING STA CONNECT/AP ATTEMPT)
    oledDisplay.printMessage("WiFi", "Starting...");
    wifiHandler.startConnect();

    // 4. Initialize Sensor Mock/Serial
    pmSensor.begin(SENSOR_BAUD_RATE);
    
    // 5. Initialize DHT22 Sensor
    dhtSensor.setup();
    
    // 6. Record time after initial setup
    firstRunTime = millis();
}

void loop() {
    // 1. Handle Wi-Fi Connection State
    wifiHandler.handleConnect();

    // Determine connection status based on Station (client) mode only
    bool currentlyConnected = (WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_STA);

    // 2. Standby Mode Command Execution (5 seconds after start)
    if (!standbyModeSet && (millis() - firstRunTime >= STANDBY_MODE_DELAY_MS)) {
        pmSensor.enterStandbyMode();
        standbyModeSet = true;
    }

    // 3. OTA Handling (Skipped for brevity)

    // 4. Blynk Data Transmission and Sensor Reading
    if (millis() - lastSendTime > BLYNK_SEND_INTERVAL_MS) {
        
        bool sensorDataAvailable = false;
        
        // --- WAKE UP, READ, and SLEEP Logic ---
        if (standbyModeSet) {
            // A. Wake up the sensor
            pmSensor.enterNormalMode();
            // B. Wait for stability (30 seconds recommended, but shortened for testing)
            delay(3000); // 3 seconds delay for initial stability check
            
            // C. Read the data (assuming Auto mode now sends a burst of data)
            sensorDataAvailable = pmSensor.readData(pm1_0_val, pm2_5_val, pm10_0_val, USE_MOCK_DATA);
            
            // D. Put the sensor back to sleep
            pmSensor.enterStandbyMode();
        } else {
            // If standby hasn't been set yet (initial 5s window), just read continuously
            sensorDataAvailable = pmSensor.readData(pm1_0_val, pm2_5_val, pm10_0_val, USE_MOCK_DATA);
        }
        
        // --- DHT Reading (Unchanged) ---
        float h = dhtSensor.readHumidity();
        float t = dhtSensor.readTemperature();

        // Check if data is available and connected before sending
        if (sensorDataAvailable && currentlyConnected) {
            // *** BLYNK UPDATE ***
            blynkHandler.sendData(BLYNK_AUTH_TOKEN, pm1_0_val, pm2_5_val, pm10_0_val, t, h);
            lastSendTime = millis();
        }
        
        // 5. Update Statuses and Display
        String wifiStatusStr = wifiHandler.getWifiStatus();
        rgbLEDHandler.updateLED(pm2_5_val, sensorDataAvailable);

        oledDisplay.displaySensorDataAndWifiStatus(wifiStatusStr, pm1_0_val, pm2_5_val, pm10_0_val, h, t);

        // 6. Serial Debug Output
        if (sensorDataAvailable) {
            Serial.print("Data Read (Mock="); Serial.print(USE_MOCK_DATA ? "T" : "F");
            Serial.print("): PM2.5="); Serial.println(pm2_5_val);
        } else {
            Serial.println("PM sensor data not available.");
        }
            
        if (!isnan(h) && !isnan(t)) {
            String tempStr = "T: " + String(t, 1) + "C";
            String humStr = "H: " + String(h, 0) + "%";
            Serial.println(tempStr);
            Serial.println(humStr);
        } else {
            Serial.println("DHT Sensor read failed.");
        }
    }
    
    // 7. Loop Delay
    delay(LOOP_DELAY);
}
