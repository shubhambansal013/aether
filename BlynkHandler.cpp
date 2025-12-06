// BlynkHandler.cpp

#include "BlynkHandler.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// Define the Blynk server for the HTTP API (must match your cloud or local server)
#define BLYNK_SERVER_HOST "blynk-cloud.com" 

// Constructor (simplified)
BlynkHandler::BlynkHandler() {
    // No dynamic allocation needed for BlynkTimer anymore
}

// Function to connect to Wi-Fi
void BlynkHandler::begin(const char* ssid, const char* pass) {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

// Simplified run function - just runs basic WiFi checks if needed, or can be left empty
void BlynkHandler::run() {
    // With HTTP API, we don't need a constant loop runner like Blynk.run()
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected. Reconnecting...");
        WiFi.reconnect();
    }
}

// Function to send data using the Blynk HTTP RESTful API batch update
void BlynkHandler::sendData(const char* auth, float pm1_0, float pm2_5, float pm10_0, float temperature, float humidity) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Cannot send data: WiFi not connected.");
        return;
    }

    WiFiClient client;
    HTTPClient http;

    // 1. Construct the URL for the batch update:
    // Format: /update/AUTH_TOKEN?V0=value0&V1=value1&V2=value2...
    String url = "http://";
    url += BLYNK_SERVER_HOST;
    url += "/external/api/batch/update?token=";
    url += auth;

    // Append all Virtual Pin data (formatted to one decimal place for floats)
    url += "&V0=";
    url += String(pm1_0, 1);
    url += "&V1=";
    url += String(pm2_5, 1);
    url += "&V2=";
    url += String(pm10_0, 1);
    url += "&V4=";
    url += String(temperature, 1);
    url += "&V5=";
    url += String(humidity, 1);

    Serial.print("Sending Batch Update: ");
    Serial.println(url);

    // 2. Begin the HTTP connection
    http.begin(client, url);

    // 3. Send the HTTP GET request
    int httpCode = http.GET();

    // 4. Check for errors and close the connection
    if (httpCode > 0) {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK) {
            Serial.println("Data sent successfully (HTTP 200).");
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
}
