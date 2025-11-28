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

### PMSA003 Air Quality Sensor

*   **RX (ESP8266):** D1 (GPIO5) - *Connected to PM Sensor TX*
*   **TX (ESP8266):** D2 (GPIO4) - *Connected to PM Sensor RX*

### Other

*   **LED Pin:** D4 (GPIO2) - *Note: This pin is also used for the DHT22. Ensure you understand the implications or use a different pin for the LED if conflicts arise.*

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

## Setup and Upload

1.  **Install Libraries:** Open your Arduino IDE, go to `Sketch > Include Library > Manage Libraries...` and search for and install the required libraries listed above.
2.  **Board Selection:** Select `NodeMCU 1.0 (ESP-12E Module)` from `Tools > Board`.
3.  **Port Selection:** Select the correct serial port for your ESP8266 from `Tools > Port`.
4.  **Blynk Configuration:** Open `blynk_config.h` and update `BLYNK_AUTH_TOKEN` with your token from the Blynk app.
5.  **GitHub OTA Configuration (Optional):** Update `GITHUB_REPO_USER`, `GITHUB_REPO_NAME`, and `FIRMWARE_BIN_NAME` in `airmon.ino` for OTA updates if you plan to use this feature.
6.  **Upload:** Click the `Upload` button in the Arduino IDE to compile and upload the sketch to your ESP8266 board.

## Usage

Once uploaded, the ESP8266 will attempt to connect to Wi-Fi. If it's the first time or if connection fails, it will set up an Access Point (AP) named `AutoConnectAP`. Connect to this AP, and a captive portal will open to configure your Wi-Fi credentials.

After successful Wi-Fi connection, the device will:

*   Initialize the PMSA003, DHT22, and OLED display.
*   Attempt to connect to Blynk.
*   Display temperature, humidity, and PM2.5 on the OLED screen.
*   Send sensor data to Blynk.
*   Perform periodic internet connectivity checks.
*   Check for OTA updates from GitHub.
