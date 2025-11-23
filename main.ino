// aqi_monitor.ino (Main Sketch File)

#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <Pinger.h>
#include <EEPROM.h>              // Required for ResetHandler
#include "WiFiHandler.h"
#include "ResetHandler.h"        // <<<--- RE-INTEGRATED
#include "PMSensor.h"

// ----------------------------------------------------------------------
// ⚠️ ARDUINO IOT CLOUD LIBRARIES ⚠️
// ----------------------------------------------------------------------
#include "thingProperties.h"
#include <ArduinoIoTCloud.h>

// ----------------------------------------------------------------------
// Added for OTA (modular handler)
// ----------------------------------------------------------------------
#include "OTAHandler.h"

// ----------------------------------------------------------------------
// ⚙️ FIRMWARE VERSION & SYSTEM CONSTANTS ⚙️
// ----------------------------------------------------------------------
const char* FIRMWARE_VERSION = "V1.0.8 - Full Modular Build (OTA Modular)";
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
const unsigned long CLOUD_SEND_INTERVAL_MS = 2000L;
const bool USE_MOCK_DATA = true;
// ----------------------------------------------------------------------

// OTA Configuration
// Change this password before exposing your device on an untrusted network
const char* OTA_PASSWORD = "changeme"; // <-- change before public use

// --- Global Variables/Instances ---
WiFiHandler wifiHandler;
// Pass the wifiHandler reference to the ResetHandler constructor
ResetHandler resetHandler(wifiHandler); // <<<--- RE-INTEGRATED
Pinger pinger;
PMSensor pmSensor(SENSOR_RX_PIN, SENSOR_TX_PIN);
OTAHandler otaHandler;

// Cloud Variables
extern float pm1_0_val;
extern float pm2_5_val;
extern float pm10_0_val;

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
    resetHandler.checkPowerCycles(); // <<<--- RE-INTEGRATED

    // 2. Initialize Cloud Properties
    initProperties();

    // 3. Initialize Wi-Fi
    if (!wifiHandler.connect(QUICK_CONNECT_TIMEOUT_MS, CONFIG_AP_TIMEOUT_SEC)) {
        Serial.println("FATAL ERROR: Wi-Fi setup failed and timed out. Rebooting...");
        delay(WIFI_FAIL_REBOOT_DELAY_MS);
        ESP.reset();
    }

    // Connection successful
    isConnected = true;
    Serial.println("Wi-Fi connected successfully! Starting Cloud...");

    // 4. Connect to Arduino Cloud
    ArduinoCloud.begin(ArduinoIoTPreferredConnection);
    ArduinoCloud.printDebugInfo();

    // 4.5 Initialize OTA (requires Wi-Fi to be connected)
    otaHandler.begin(OTA_PASSWORD, "airmon");

    // 5. Initialize Sensor Mock/Serial
    pmSensor.begin(SENSOR_BAUD_RATE);
}

void loop() {
    // 1. Update Cloud Connection
    ArduinoCloud.update();

    // 1.5 Handle OTA (keep after cloud update so OTA is responsive)
    otaHandler.handle();

    // 2. Update LED Status
    updateLED();

    // 3. Run Sensor and Cloud Update Timer
    if (millis() - lastSendTime > CLOUD_SEND_INTERVAL_MS) {
        
        if (pmSensor.readData(pm1_0_val, pm2_5_val, pm10_0_val, USE_MOCK_DATA)) {
            Serial.print("Data Read (Mock="); Serial.print(USE_MOCK_DATA ? "T" : "F");
            Serial.print("): PM2.5="); Serial.println(pm2_5_val);
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