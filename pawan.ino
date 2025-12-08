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
const int WIFI_FAIL_REBOOT_DELAY_MS = 3000;
const int SETUP_DELAY_MS = 100;

// --- Wi-Fi Connection Constants ---
const unsigned long QUICK_CONNECT_TIMEOUT_MS = 180000;
const int CONFIG_AP_TIMEOUT_SEC = 600;

// --- LED/Diagnostic Constants ---
const IPAddress PING_TARGET(8, 8, 8, 8);
const long PING_INTERVAL_MS = 10000;

// --- Sensor Constants ---
const long SENSOR_BAUD_RATE = 9600;
const unsigned long BLYNK_SEND_INTERVAL_MS = 60000L;
const bool USE_MOCK_DATA = false;

// --- Loop delay ---
const long LOOP_DELAY = 1000;
// ----------------------------------------------------------------------

// --- OTA Update Constants ---
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

unsigned long lastPingTime = 0;
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

    // 2. Check for Power Cycle Reset (Must be run before Wi-Fi)
    resetHandler.checkPowerCycles();

    // 3. Initialize Wi-Fi (STARTS NON-BLOCKING STA CONNECT ATTEMPT)
    oledDisplay.printMessage("WiFi", "Starting...");
    wifiHandler.startConnect(QUICK_CONNECT_TIMEOUT_MS, (unsigned long)CONFIG_AP_TIMEOUT_SEC);

    // 6. Initialize Sensor Mock/Serial
    pmSensor.begin(SENSOR_BAUD_RATE);

    // 7. Initialize DHT22 Sensor
    dhtSensor.setup();
    
    // ⚠️ CRITICAL FIX: REMOVE BLOCKING DELAY/MESSAGE HERE
    // Execution must proceed directly to loop() for sensor data display.
}

void loop() {
    // 1. Handle Wi-Fi Connection State
    // This is where the AP portal is initiated non-blocking if STA fails.
    wifiHandler.handleConnect();

    // Determine connection status based on Station (client) mode only
    bool currentlyConnected = (WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_STA);

    // Initialize OTA once WiFi is connected
    // if (currentlyConnected && !_otaInitialized) {
    //     Serial.println("Wi-Fi connected successfully! Initializing OTA...");
    //
    //     otaHandler.setupArduinoOTA();
    //     _otaInitialized = true;
    // }

    // 2. Handle ArduinoOTA events (only if initialized)
    // if (_otaInitialized) {
    //     otaHandler.handleArduinoOTA();
    // }

    // 4. Read Sensor Data (Runs every loop regardless of WiFi status)
    bool sensorDataAvailable = pmSensor.readData(pm1_0_val, pm2_5_val, pm10_0_val, USE_MOCK_DATA);
    // Read DHT22 data
    float h = dhtSensor.readHumidity();
    float t = dhtSensor.readTemperature();

    // 5. Update Statuses
    String wifiStatusStr = wifiHandler.getWifiStatus();
    
    rgbLEDHandler.updateLED(currentlyConnected, pm2_5_val, sensorDataAvailable);

    // Always attempt to display PM data, and DHT data if available
    oledDisplay.displaySensorDataAndWifiStatus(wifiStatusStr, pm1_0_val, pm2_5_val, pm10_0_val, h, t);

    if (millis() - lastSendTime > BLYNK_SEND_INTERVAL_MS) {
        if (sensorDataAvailable && currentlyConnected) {
            // *** BLYNK UPDATE ***
            blynkHandler.sendData(BLYNK_AUTH_TOKEN, pm1_0_val, pm2_5_val, pm10_0_val, t, h);
            lastSendTime = millis();
        }
    }
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
    delay(LOOP_DELAY);
}
