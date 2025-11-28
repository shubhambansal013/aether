// aqi_monitor.ino (Main Sketch File)

#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <Pinger.h>
#include <EEPROM.h>
#include "WiFiHandler.h"
#include "ResetHandler.h"
#include "PMSensor.h"
#include "BlynkHandler.h"
#include "blynk_config.h"
#include "OTAHandler.h" // Include OTAHandler

// ----------------------------------------------------------------------
// ⚙️ FIRMWARE VERSION & SYSTEM CONSTANTS ��️
// ----------------------------------------------------------------------
const char* FIRMWARE_VERSION = "V1.0.9 - Blynk Integration";
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
const int SENSOR_RX_PIN = 5; // D1 (GPIO 5)
const int SENSOR_TX_PIN = 4; // D2 (GPIO 4)
const unsigned long BLYNK_SEND_INTERVAL_MS = 2000L;
const bool USE_MOCK_DATA = true;
// ----------------------------------------------------------------------

// --- OTA Update Constants ---
// !!! IMPORTANT: Replace with your GitHub repository details !!!
const char* GITHUB_REPO_USER = "your-github-username";
const char* GITHUB_REPO_NAME = "your-repo-name";
const char* FIRMWARE_BIN_NAME = "firmware.bin"; // Name of the .bin file in your GitHub release assets
// ----------------------------------------------------------------------

// --- Global Variables/Instances ---
WiFiHandler wifiHandler;
ResetHandler resetHandler(wifiHandler);
Pinger pinger;
PMSensor pmSensor(SENSOR_RX_PIN, SENSOR_TX_PIN);
BlynkHandler blynkHandler;
OTAHandler otaHandler(FIRMWARE_VERSION, GITHUB_REPO_USER, GITHUB_REPO_NAME, FIRMWARE_BIN_NAME); // OTA Handler instance

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
bool isConnected = false;
unsigned long lastSendTime = 0;

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
    if (!isConnected) {
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

    // 1. Check for Power Cycle Reset (Must be run before Wi-Fi)
    resetHandler.checkPowerCycles();

    // 2. Initialize Wi-Fi
    if (!wifiHandler.connect(QUICK_CONNECT_TIMEOUT_MS, CONFIG_AP_TIMEOUT_SEC)) {
        Serial.println("FATAL ERROR: Wi-Fi setup failed and timed out. Rebooting...");
        delay(WIFI_FAIL_REBOOT_DELAY_MS);
        ESP.reset();
    }

    // Connection successful
    isConnected = true;
    Serial.println("Wi-Fi connected successfully! Starting Blynk...");

    // 3. Check for and perform OTA updates
    otaHandler.checkAndUpdate();

    // 4. Connect to Blynk
    blynkHandler.begin(BLYNK_AUTH_TOKEN, WiFi.SSID().c_str(), WiFi.psk().c_str());
    blynkHandler.sendFirmwareVersion(FIRMWARE_VERSION);

    // 5. Initialize Sensor Mock/Serial
    pmSensor.begin(SENSOR_BAUD_RATE);
}

void loop() {
    // 1. Run Blynk
    blynkHandler.run();

    // 2. Update LED Status
    updateLED();

    // 3. Run Sensor and Blynk Update Timer
    if (millis() - lastSendTime > BLYNK_SEND_INTERVAL_MS) {

        if (pmSensor.readData(pm1_0_val, pm2_5_val, pm10_0_val, USE_MOCK_DATA)) {
            Serial.print("Data Read (Mock="); Serial.print(USE_MOCK_DATA ? "T" : "F");
            Serial.print("): PM2.5="); Serial.println(pm2_5_val);
            blynkHandler.sendSensorData(pm1_0_val, pm2_5_val, pm10_0_val);
        } else {
            Serial.println("Sensor read failed (if not mocking).");
        }

        lastSendTime = millis();
    }

    // 4. Run Ping Diagnostic
    if (isConnected && millis() - lastPingTime >= PING_INTERVAL_MS) {
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
