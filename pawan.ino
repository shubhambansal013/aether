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
const bool USE_MOCK_DATA = false;

// --- Reading Cycle Constants (All Configurable) ---
const unsigned long INITIAL_AUTO_DELAY_MS = 5000L;    // 1. Initial 5s delay before Passive Mode (X=5s)
const unsigned long ACTIVE_READ_DURATION_MS = 5000L; // 2. Total reading duration after wake-up (20s)
const unsigned long STABILITY_TIME_MS = 5000L;        // 3. Time required for data to stabilize after wake-up (5s)
const unsigned long SLEEP_DURATION_MS = 5000L;      // 4. Sleep duration (2 minutes = 120s)

// --- Loop delay ---
const long LOOP_DELAY = 1000;

// --- Blynk Timing Control ---
unsigned long lastBlynkSendTime = 0;
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

// Sensor Mode Management
enum SensorMode { 
    MODE_AUTO,          // A - Initial mode (before 5s)
    MODE_PASSIVE_INIT,  // P - Initial Passive command sent (unused in final logic)
    MODE_READING,       // R - Reading actively for 20s
    MODE_SLEEP          // S - Sleeping for 2 minutes
};
SensorMode currentSensorMode = MODE_AUTO; // Start in Auto Mode

unsigned long modeStartTime = 0;
float pm1_0_val = 0.0;
float pm2_5_val = 0.0;
float pm10_0_val = 0.0;
bool blynkSendPending = false; // Flag to ensure Blynk is sent exactly once per read cycle
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
    rgbLEDHandler.startupSequence();

    // 2. Check for Power Cycle Reset
    resetHandler.checkPowerCycles();

    // 3. Initialize Wi-Fi
    oledDisplay.printMessage("WiFi", "Starting...");
    wifiHandler.startConnect();

    // 4. Initialize Sensor Serial
    pmSensor.begin(SENSOR_BAUD_RATE);
    
    // 5. Initialize DHT22 Sensor
    dhtSensor.setup();
    
    // Set initial mode start time
    modeStartTime = millis();
}

/**
 * @brief Maps the current SensorMode to a single character for display.
 */
String getSensorModeChar(SensorMode mode) {
    switch (mode) {
        case MODE_AUTO: return "A";
        case MODE_PASSIVE_INIT: return "P";
        case MODE_READING: return "R";
        case MODE_SLEEP: return "S";
        default: return "?";
    }
}

/**
 * @brief Manages the PMSensor state machine.
 */
void handleSensorState() {
    unsigned long currentDuration = millis() - modeStartTime;
    bool dataWasRead = false;
    
    switch (currentSensorMode) {
        
        // 1. MODE_AUTO: Initial state, waits for INITIAL_AUTO_DELAY_MS
        case MODE_AUTO:
            // Read data during the initial period for display
            pmSensor.readData(pm1_0_val, pm2_5_val, pm10_0_val, USE_MOCK_DATA);
            
            if (currentDuration >= INITIAL_AUTO_DELAY_MS) {
                // Initialize Passive Mode and Standby for the cycle
                pmSensor.switchToPassiveMode(); 
                pmSensor.enterStandbyMode();    
                currentSensorMode = MODE_SLEEP;
                modeStartTime = millis();
                Serial.println("--- State Change: AUTO -> SLEEP (Initial Passive Mode Set) ---");
            }
            break;

        // 2. MODE_SLEEP: Sensor is asleep, waiting for SLEEP_DURATION_MS
        case MODE_SLEEP:
            if (currentDuration >= SLEEP_DURATION_MS) {
                // Time to wake up and read
                pmSensor.enterNormalMode(); 
                
                // Sensor is now awake and in Passive Mode.
                currentSensorMode = MODE_READING;
                modeStartTime = millis();
                blynkSendPending = true; 
                Serial.println("--- State Change: SLEEP -> READING (Waking up, staying in Passive Mode) ---");
            }
            break;

        // 3. MODE_READING: Sensor is active, polling for data for ACTIVE_READ_DURATION_MS
        case MODE_READING:
            
            // --- DATA REQUEST ---
            // Request data periodically (every loop, since LOOP_DELAY=1000) and wait briefly.
            pmSensor.requestData(); 
            // CRITICAL FIX: Give the sensor a moment to transmit the packet header.
            delay(10); 

            // Check for new data packet received
            dataWasRead = pmSensor.readData(pm1_0_val, pm2_5_val, pm10_0_val, USE_MOCK_DATA);
            
            // --- STABILITY CHECK AND BLYNK TRIGGER ---
            if (dataWasRead && blynkSendPending && (currentDuration >= STABILITY_TIME_MS)) {
                // Data is fresh, ready to send, and stable.
                lastBlynkSendTime = 0; // Trigger Blynk send in loop()
                blynkSendPending = false; // Clear flag until next sleep cycle
                Serial.println("--- Stable Data Acquired. Triggering Blynk Send. ---");
            }

            if (currentDuration >= ACTIVE_READ_DURATION_MS) {
                // Time to switch back to standby
                pmSensor.enterStandbyMode();
                currentSensorMode = MODE_SLEEP;
                modeStartTime = millis();
                Serial.println("--- State Change: READING -> SLEEP (Reading Complete) ---");
            }
            break;
            
        case MODE_PASSIVE_INIT:
            break;
    }
}


void loop() {
    // 1. Handle Sensor State Machine
    handleSensorState();

    // 2. Handle Wi-Fi Connection State
    wifiHandler.handleConnect();

    // Determine connection status based on Station (client) mode only
    bool currentlyConnected = (WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_STA);

    // 3. OTA Handling (Skipped for brevity)

    // 4. Blynk Data Transmission (Triggered by state machine via lastBlynkSendTime = 0)
    if (lastBlynkSendTime == 0) {
        
        bool sensorDataValid = (pm2_5_val > 0.0); // Check if we have received non-zero data
        float h = dhtSensor.readHumidity();
        float t = dhtSensor.readTemperature();

        if (sensorDataValid && currentlyConnected) {
            // *** BLYNK UPDATE ***
            blynkHandler.sendData(BLYNK_AUTH_TOKEN, pm1_0_val, pm2_5_val, pm10_0_val, t, h);
            lastBlynkSendTime = millis();
            Serial.println("BLYNK: Stable data sent successfully.");
        } else {
            // If send was triggered but connection/data failed, log it and prevent immediate re-trigger
            lastBlynkSendTime = millis(); 
            Serial.println("BLYNK: Send triggered, but skipping (Wi-Fi or data error).");
        }
    }
    
    // 5. Read DHT Data (Run every loop to get the latest values for display)
    float h = dhtSensor.readHumidity();
    float t = dhtSensor.readTemperature();

    // 6. Update Statuses and Display
    String wifiStatusStr = wifiHandler.getWifiStatus();
    
    rgbLEDHandler.updateLED(pm2_5_val, currentSensorMode != MODE_SLEEP); // LED reflects active/sleep state
    
    // Call the updated OLED function
    oledDisplay.displaySensorDataAndWifiStatus(getSensorModeChar(currentSensorMode), wifiStatusStr, pm1_0_val, pm2_5_val, pm10_0_val, h, t);

    // 7. Serial Debug Output (Simplified)
    if (currentSensorMode != MODE_SLEEP) {
        Serial.print("PM2.5: "); Serial.println(pm2_5_val);
    }
    if (!isnan(h) && !isnan(t)) {
        Serial.print("T/H: "); Serial.print(t, 1); Serial.print("C / "); Serial.print(h, 0); Serial.println("%");
    }
    
    delay(LOOP_DELAY);
}
