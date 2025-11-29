#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <Pinger.h>
#include <EEPROM.h>
#include "WiFiHandler.h"
#include "ResetHandler.h"
#include "PMSensor.h"
#include "BlynkHandler.h"
#include "blynk_config.h"
#include "OTAHandler.h" 
#include "DHTSensor.h" 
#include "OLEDDisplay.h" 

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
const int LED_PIN = D4;
const IPAddress PING_TARGET(8, 8, 8, 8);
const long PING_INTERVAL_MS = 10000;

// --- Sensor Constants ---
const long SENSOR_BAUD_RATE = 9600;

// *** PIN ASSIGNMENT FIX ***
// Old Config: D1/D2 (Conflicted with OLED)
// New Config: D5/D6
const int SENSOR_RX_PIN = D5; // GPIO 14
const int SENSOR_TX_PIN = D6; // GPIO 12

const unsigned long BLYNK_SEND_INTERVAL_MS = 2000L;
const bool USE_MOCK_DATA = true;
// ----------------------------------------------------------------------

// --- OTA Update Constants ---
const char* GITHUB_REPO_USER = "shubhambansal013";
const char* GITHUB_REPO_NAME = "airmon";
const char* FIRMWARE_BIN_NAME = "firmware.bin"; 
// ----------------------------------------------------------------------

// --- Global Variables/Instances ---
WiFiHandler wifiHandler;
ResetHandler resetHandler(wifiHandler);
Pinger pinger;
PMSensor pmSensor(SENSOR_RX_PIN, SENSOR_TX_PIN);
BlynkHandler blynkHandler;
OTAHandler otaHandler(FIRMWARE_VERSION, GITHUB_REPO_USER, GITHUB_REPO_NAME, FIRMWARE_BIN_NAME); 
DHTSensor dhtSensor;
OLEDDisplay oledDisplay;

// Cloud Variables
float pm1_0_val;
float pm2_5_val;
float pm10_0_val;

// LED control variables
unsigned long lastPingTime = 0;
unsigned long ledStateTime = 0;
bool ledState = HIGH;
int blinkCount = 0;
bool isBlinking = false;
// Removed isConnected
unsigned long lastSendTime = 0;
bool _blynkAndOtaInitialized = false;

// --- Diagnostic Status Codes ---
enum DiagnosticStatus {
    STATUS_PING_SUCCESS = 1,
    STATUS_PING_FAILURE = 2
};

// ----------------------------------------------------------------------
// ⚡️ LED BLINKING LOGIC & HELPER FUNCTIONS ⚡️
// ----------------------------------------------------------------------

void startBlink(DiagnosticStatus status) {
    if (isBlinking) return;

    blinkCount = (int)status * 2;
    isBlinking = true;
    ledStateTime = millis();
    digitalWrite(LED_PIN, LOW); // Start with LED ON (Active LOW)
    ledState = LOW;
}

void updateLED() {
    if (WiFi.status() != WL_CONNECTED) {
        // Continuous ON when trying to connect (LED solid LOW)
        digitalWrite(LED_PIN, LOW);
        return;
    }

    if (!isBlinking) {
        // Stays OFF when successfully connected and not mid-blink (LED solid HIGH)
        digitalWrite(LED_PIN, HIGH);
        return;
    }

    // Handle blinking state
    if (millis() - ledStateTime >= 150) {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
        ledStateTime = millis();
        blinkCount--;

        if (blinkCount <= 0) {
            isBlinking = false;
        }
    }
}

// ----------------------------------------------------------------------
// SETUP & LOOP
// ----------------------------------------------------------------------

void setup() {
    Serial.begin(DEBUG_BAUD_RATE);
    pinMode(LED_PIN, OUTPUT);
    delay(SETUP_DELAY_MS);
    Serial.print("\nFirmware Version: ");
    Serial.println(FIRMWARE_VERSION);

    // Initial LED State: Solid ON (LOW)
    digitalWrite(LED_PIN, LOW);

    // 1. Initialize OLED Display FIRST to show status during boot
    // Moving this up helps debug boot issues visually
    oledDisplay.setup();
    oledDisplay.printMessage("System", "Booting...");

    // 2. Check for Power Cycle Reset (Must be run before Wi-Fi)
    resetHandler.checkPowerCycles();

    // 3. Initialize Wi-Fi
    oledDisplay.printMessage("WiFi", "Starting...");
    wifiHandler.startConnect(QUICK_CONNECT_TIMEOUT_MS, CONFIG_AP_TIMEOUT_SEC);

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

    // 4. Update LED Status
    updateLED();

    // 5. Run Sensor and Blynk Update Timer
    if (millis() - lastSendTime > BLYNK_SEND_INTERVAL_MS) {

        if (pmSensor.readData(pm1_0_val, pm2_5_val, pm10_0_val, USE_MOCK_DATA)) {
            Serial.print("Data Read (Mock="); Serial.print(USE_MOCK_DATA ? "T" : "F");
            Serial.print("): PM2.5="); Serial.println(pm2_5_val);
            if (_blynkAndOtaInitialized && currentlyConnected) {
                blynkHandler.sendSensorData(pm1_0_val, pm2_5_val, pm10_0_val);
            }
        } else {
            Serial.println("Sensor read failed (if not mocking).");
        }

        // Read DHT22 data and display on OLED
        float h = dhtSensor.readHumidity();
        float t = dhtSensor.readTemperature();

        String wifiStatusStr = wifiHandler.getWifiStatus();

        if (!isnan(h) && !isnan(t)) {
            String tempStr = "T: " + String(t, 1) + "C";
            String humStr = "H: " + String(h, 0) + "%";
            String pm25Str = "PM2.5: " + String(pm2_5_val, 0); // Removed decimals for space
            
            oledDisplay.displaySensorDataAndWifiStatus(wifiStatusStr, tempStr, humStr, pm25Str);
            
            Serial.println(tempStr);
            Serial.println(humStr);
            if (_blynkAndOtaInitialized && currentlyConnected) {
                blynkHandler.sendTemperatureHumidity(t, h);
            }
        } else {
            // If DHT fails, still show PM data
            String pm25Str = "PM2.5: " + String(pm2_5_val, 0);
            oledDisplay.displaySensorDataAndWifiStatus(wifiStatusStr, "DHT Fail", pm25Str, ""); // Pass empty string for line3
        }

        lastSendTime = millis();
    }

    // 6. Run Ping Diagnostic (only if connected)
    if (currentlyConnected && millis() - lastPingTime >= PING_INTERVAL_MS) {
        lastPingTime = millis();
        Serial.print("\n## Pinging Internet Target (8.8.8.8)... ");

        if (pinger.Ping(PING_TARGET)) {
            Serial.println("SUCCESS: Internet access confirmed!");
            startBlink(STATUS_PING_SUCCESS);
        } else {
            Serial.println("FAILURE: Ping failed. Router or ISP connection issue.");
            startBlink(STATUS_PING_FAILURE);
        }
    }
}
