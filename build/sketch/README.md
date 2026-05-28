#line 1 "/home/runner/work/aether/aether/README.md"
# Air Quality Monitor (ESP8266 NodeMCU)

This project monitors air quality using a PMSA003 sensor, and now also includes a DHT22 temperature/humidity sensor and an OLED I2C display for local readings.

## Features

*   **Wi-Fi Connectivity:** Manages Wi-Fi connections with a captive portal for easy configuration.
*   **OTA Updates:** Supports Over-The-Air firmware updates from a GitHub repository.
*   **PMSensor Integration:** Reads PM1.0, PM2.5, and PM10.0 data.
*   **Blynk Integration:** Sends sensor data to the Blynk IoT platform.
*   **DHT22 Sensor:** Reads temperature and humidity.
*   **OLED Display:** Shows current temperature, humidity, and PM2.5 values locally.
*   **System Diagnostics:** Pings an internet target to confirm connectivity.
*   **Reset Handling:** Detects power cycles for robust operation.

## Pin Configuration

This project is designed for an ESP8266 NodeMCU board.

### DHT22 Temperature & Humidity Sensor

*   **Data Pin:** D4 (GPIO2)

### OLED I2C Display (SSD1306 128x64)

*   **SDA (Data):** D1 (GPIO5)
*   **SCL (Clock):** D2 (GPIO4)

### RGB LED (Common Cathode)

A 4-pin common cathode RGB LED is used for a rough visual indicator of device status and air quality. Each color pin (R, G, B) should be connected in series with a 220-ohm current-limiting resistor to the specified ESP8266 GPIO pin. The common cathode pin should be connected to ESP8266 GND.

*   **Red Pin (via resistor):** D0 (GPIO16)
*   **Green Pin (via resistor):** D3 (GPIO0)
*   **Blue Pin (via resistor):** D7 (GPIO13)

#### RGB LED Indicator Scheme:

*   **Pulsing Blue:** Initial setup, Wi-Fi connecting, or in AP configuration mode. Indicates the device is busy with network tasks.
*   **Solid Green:** Wi-Fi connected, and PM2.5 levels are good (< 12 µg/m³).
*   **Solid Yellow/Amber (Red + Green):** Wi-Fi connected, and PM2.5 levels are moderate (12 µg/m³ <= PM2.5 < 35 µg/m³).
*   **Solid Red:** Wi-Fi connected, and PM2.5 levels are bad (>= 35 µg/m³).
*   **Solid Magenta (Red + Blue):** Wi-Fi connected, but sensor data is unavailable or there's a sensor error.
*   **Blinking Green:** Short blinks to indicate a successful internet ping.
*   **Blinking Red:** Short blinks to indicate a failed internet ping.

### PMSA003 Air Quality Sensor

*   **RX (ESP8266):** D1 (GPIO5) - *Connected to PM Sensor TX*
*   **TX (ESP8266):** D2 (GPIO4) - *Connected to PM Sensor RX*

### Other

*   **Onboard LED:** D4 (GPIO2) - *This pin is still used by the DHT22 sensor; its original purpose as a general status LED has been replaced by the RGB LED. No changes are needed here, but be aware of its dual role if troubleshooting.*

## Libraries Used

Ensure you have these libraries installed in your Arduino IDE:

*   `ESP8266WiFi` (Built-in)
*   `WiFiManager` by Tzapu
*   `Pinger` by ctz
*   `EEPROM` (Built-in)
*   `Blynk` by Blynk Community
*   `DHT sensor library` by Adafruit
*   `Adafruit GFX Library` by Adafruit
*   `Adafruit SSD1306` by Adafruit

## Setup and Upload (Initial Flash via Serial)

1.  **Install Libraries:** Ensure you have the required libraries (listed above) installed for your Arduino IDE or `arduino-cli`. For `arduino-cli`, use `arduino-cli lib install "Library Name"`.
2.  **Board Selection:** For Arduino IDE, select `NodeMCU 1.0 (ESP-12E Module)` from `Tools > Board`.
3.  **Blynk Configuration:** Open `blynk_config.h` and update `BLYNK_AUTH_TOKEN` with your token from the Blynk app.
4.  **GitHub OTA Configuration (Optional):** If you plan to use the GitHub release-based OTA feature, update `GITHUB_REPO_USER`, `GITHUB_REPO_NAME`, and `FIRMWARE_BIN_NAME` in `airmon.ino`.
5.  **Upload via Serial:**
    *   **Arduino IDE:** Select the correct serial port and click `Upload`.
    *   **arduino-cli:**
        ```bash
        arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 airmon.ino
        arduino-cli upload -p /dev/ttyUSB0 --fqbn esp8266:esp8266:nodemcuv2 airmon.ino
        ```
        (Replace `/dev/ttyUSB0` with your ESP8266's serial port.)

## Usage

### Wi-Fi Configuration with WiFiManager

On first boot or if previous credentials fail, the ESP8266 will create an Access Point (AP) named `airmon_AP` (password `password`). Connect to this AP from your phone or computer, and a captive portal will appear, allowing you to enter your Wi-Fi network credentials. These credentials will be saved for future boots.

### Over-The-Air (OTA) Updates

This project supports two methods for OTA updates:

#### 1. GitHub Release-based OTA (Configured in `OTAHandler.cpp`)

The device can check for and download new firmware from a specified GitHub repository release. This is configured via `GITHUB_REPO_USER`, `GITHUB_REPO_NAME`, and `FIRMWARE_BIN_NAME` constants in `airmon.ino`. The `otaHandler.checkAndUpdate()` method would trigger this (currently commented out in `airmon.ino`'s `setup()` to prioritize `ArduinoOTA`).

#### 2. Local Network OTA using `arduino-cli` (via `ArduinoOTA` library)

Once the device is connected to your Wi-Fi network (managed by `WiFiManager`), it will listen for OTA updates from your local machine using the `ArduinoOTA` library.

**Steps for `arduino-cli` OTA Update:**

1.  **Ensure Initial Flash:** The device must have been flashed *at least once via serial* with an OTA-enabled sketch (as per "Setup and Upload" above).
2.  **Get Device IP:** Monitor the serial output during boot; the ESP8266 will print its assigned IP address after connecting to Wi-Fi (e.g., `IP address: 192.168.1.100`). The device will also be discoverable on the network as `airmon-esp8266`.
3.  **Compile New Sketch:** From your project directory:
    ```bash
    arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 airmon.ino
    ```
4.  **Upload via Network:**
    ```bash
    arduino-cli upload --fqbn esp8266:esp8266:nodemcuv2 --input-dir . --port <ESP_IP_ADDRESS> --protocol network
    ```
    (Replace `<ESP_IP_ADDRESS>` with the actual IP address of your ESP8266 or `airmon-esp8266.local` if mDNS is working.)

### Normal Operation

After successful Wi-Fi connection and potential OTA updates, the device will:

*   Initialize PMSA003, DHT22, and OLED display.
*   Connect to the Blynk IoT platform.
*   Display temperature, humidity, and PM2.5 on the OLED screen.
*   Send sensor data to Blynk every `BLYNK_SEND_INTERVAL_MS` (2 seconds).
*   Perform periodic internet connectivity checks (ping `8.8.8.8`).
