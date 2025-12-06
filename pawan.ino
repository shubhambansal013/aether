#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <Pinger.h>
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
#include "RGBLEDHandler.h" // New include

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
// No longer need LED_PIN constant here, it's managed by RGBLEDHandler
const IPAddress PING_TARGET(8, 8, 8, 8);
const long PING_INTERVAL_MS = 10000;

// --- Sensor Constants ---
const long SENSOR_BAUD_RATE = 9600;
const unsigned long BLYNK_SEND_INTERVAL_MS = 2000L;
const bool USE_MOCK_DATA = false;
// ----------------------------------------------------------------------

// --- OTA Update Constants ---
const char* GITHUB_REPO_USER = "shubhambansal013";
const char* GITHUB_REPO_NAME = "pawan";
const char* FIRMWARE_BIN_NAME = "firmware.bin"; 
// ----------------------------------------------------------------------

// --- Global Variables/Instances ---
WiFiHandler wifiHandler;
ResetHandler resetHandler(wifiHandler);
Pinger pinger;
PMSensor pmSensor(D6, D5);
BlynkHandler blynkHandler;
OTAHandler otaHandler(FIRMWARE_VERSION, GITHUB_REPO_USER, GITHUB_REPO_NAME, FIRMWARE_BIN_NAME); 
DHTSensor dhtSensor;
OLEDDisplay oledDisplay;
RGBLEDHandler rgbLEDHandler(RGB_LED_RED_PIN, RGB_LED_GREEN_PIN, RGB_LED_BLUE_PIN); // New instance

// Cloud Variables
float pm1_0_val;
float pm2_5_val;
float pm10_0_val;

unsigned long lastPingTime = 0;
unsigned long lastSendTime = 0;
bool _blynkAndOtaInitialized = false;

// ----------------------------------------------------------------------
// SETUP & LOOP
// ----------------------------------------------------------------------

void setup() {
    Serial.begin(DEBUG_BAUD_RATE);
    delay(SETUP_DELAY_MS);
    Serial.print("\nFirmware Version: ");
    Serial.println(FIRMWARE_VERSION);

    // Initial LED State (handled by RGBLEDHandler.setup())
    // digitalWrite(LED_PIN, LOW); // Remove this

    // 1. Initialize OLED Display FIRST to show status during boot
    // Moving this up helps debug boot issues visually
    oledDisplay.setup();
    oledDisplay.printMessage("System", "Booting...");

    // Initialize RGB LED
    rgbLEDHandler.setup(); // New call

    // 2. Check for Power Cycle Reset (Must be run before Wi-Fi)
    resetHandler.checkPowerCycles();

    // 3. Initialize Wi-Fi
    oledDisplay.printMessage("WiFi", "Starting...");
    wifiHandler.startConnect(QUICK_CONNECT_TIMEOUT_MS, (unsigned long)CONFIG_AP_TIMEOUT_SEC);

    // 6. Initialize Sensor Mock/Serial
    pmSensor.begin(SENSOR_BAUD_RATE);

    // 7. Initialize DHT22 Sensor
    dhtSensor.setup();
    
    oledDisplay.printMessage("System", "Ready!");
    delay(1000);
}

void loop() {
    // 1. Handle Wi-Fi Connection State
    bool currentlyConnected = (WiFi.status() == WL_CONNECTED);
    wifiHandler.handleConnect();

    // Initialize Blynk and OTA once WiFi is connected
    if (currentlyConnected && !_blynkAndOtaInitialized) {
        Serial.println("Wi-Fi connected successfully! Initializing Blynk and OTA...");
        otaHandler.setupArduinoOTA();
        blynkHandler.begin(BLYNK_AUTH_TOKEN, WiFi.SSID().c_str(), WiFi.psk().c_str());
        blynkHandler.sendFirmwareVersion(FIRMWARE_VERSION);
        _blynkAndOtaInitialized = true;
    }

    // 2. Handle ArduinoOTA events (only if initialized)
    if (_blynkAndOtaInitialized) {
        otaHandler.handleArduinoOTA();
    }

    // 3. Run Blynk (only if initialized and connected)
    if (_blynkAndOtaInitialized && currentlyConnected) {
        blynkHandler.run();
    }

    // 4. Update LED Status (using RGB LED Handler)
    bool sensorDataAvailable = pmSensor.readData(pm1_0_val, pm2_5_val, pm10_0_val, USE_MOCK_DATA);
    rgbLEDHandler.updateLED(currentlyConnected, pm2_5_val, sensorDataAvailable);


    // 5. Run Sensor and Blynk Update Timer
    if (millis() - lastSendTime > BLYNK_SEND_INTERVAL_MS) {
        if (sensorDataAvailable) {
            Serial.print("Data Read (Mock="); Serial.print(USE_MOCK_DATA ? "T" : "F");
            Serial.print("): PM2.5="); Serial.println(pm2_5_val);

            if (_blynkAndOtaInitialized && currentlyConnected) {
                blynkHandler.sendSensorData(pm1_0_val, pm2_5_val, pm10_0_val);
            }

            // Read DHT22 data and display on OLED only if PM data is available
            float h = dhtSensor.readHumidity();
            float t = dhtSensor.readTemperature();
            String wifiStatusStr = wifiHandler.getWifiStatus();

            // Always attempt to display PM data, and DHT data if available
            oledDisplay.displaySensorDataAndWifiStatus(wifiStatusStr, pm1_0_val, pm2_5_val, pm10_0_val, h, t);

            if (!isnan(h) && !isnan(t)) {
                String tempStr = "T: " + String(t, 1) + "C";
                String humStr = "H: " + String(h, 0) + "%";
                
                Serial.println(tempStr);
                Serial.println(humStr);

                if (_blynkAndOtaInitialized && currentlyConnected) {
                    blynkHandler.sendTemperatureHumidity(t, h);
                }
            } else {
                Serial.println("DHT Sensor read failed.");
            }
        } else {
            Serial.println("Sensor read failed (if not mocking).");
        }

        lastSendTime = millis();
    }

    // 6. Run Ping Diagnostic (only if connected)
    if (currentlyConnected && millis() - lastPingTime >= PING_INTERVAL_MS) {
        lastPingTime = millis();
        Serial.print("\n## Pinging Internet Target (8.8.8.8)... ");

        if (pinger.Ping(PING_TARGET)) {
            Serial.println("SUCCESS: Internet access confirmed!");
            rgbLEDHandler.startBlink(RGBLEDHandler::STATUS_PING_SUCCESS); // New call
        } else {
            Serial.println("FAILURE: Ping failed. Router or ISP connection issue.");
            rgbLEDHandler.startBlink(RGBLEDHandler::STATUS_PING_FAILURE); // New call
        }
    }
delay(100);
}
