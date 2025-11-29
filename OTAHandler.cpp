
// OTAHandler.cpp
#include "OTAHandler.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ArduinoJson.h> // For parsing GitHub API response
#include <ESP8266WiFi.h> // Required for WiFi.localIP() and ArduinoOTA

// Constructor
OTAHandler::OTAHandler(const char* currentVersion, const char* repoUser, const char* repoName, const char* firmwareFile)
    : _currentVersion(currentVersion),
      _repoUser(repoUser),
      _repoName(repoName),
      _firmwareFile(firmwareFile) {
    _repoApiUrl = "https://api.github.com/repos/";
    _repoApiUrl += _repoUser;
    _repoApiUrl += "/";
    _repoApiUrl += _repoName;
}

// --- ArduinoOTA Methods ---

void OTAHandler::setupArduinoOTA() {
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_SPIFFS
            type = "filesystem";
        }
        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (1000.0 * 10.0)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });

    // Hostname defaults to esp8266-[ChipID]
    // Set a custom hostname to make it easier to find on the network
    ArduinoOTA.setHostname("airmon-esp8266");

    ArduinoOTA.begin();
    Serial.println("ArduinoOTA Initialized.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void OTAHandler::handleArduinoOTA() {
    ArduinoOTA.handle();
}

// --- GitHub-based OTA Methods (existing) ---

// Get the latest version tag from GitHub releases
String OTAHandler::getLatestVersionTag() {
    Serial.println("Checking for latest firmware version on GitHub...");
    String latestVersion = "";
    HTTPClient http;
    String url = _repoApiUrl + "/releases/latest";

    WiFiClient client;
    http.begin(client, url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("GitHub API Response:");
        Serial.println(payload);

        DynamicJsonDocument doc(1024); // Adjust size as needed
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
        } else {
            latestVersion = doc["tag_name"].as<String>();
            Serial.print("Latest version found: ");
            Serial.println(latestVersion);
        }
    } else {
        Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    return latestVersion;
}

// Perform the OTA update
void OTAHandler::performUpdate(const String& latestVersionTag) {
    Serial.println("Performing OTA update...");
    String firmwareUrl = "https://github.com/";
    firmwareUrl += _repoUser;
    firmwareUrl += "/";
    firmwareUrl += _repoName;
    firmwareUrl += "/releases/download/";
    firmwareUrl += latestVersionTag;
    firmwareUrl += "/";
    firmwareUrl += _firmwareFile;

    Serial.print("Firmware URL: ");
    Serial.println(firmwareUrl);

    WiFiClient client;
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, firmwareUrl, _currentVersion);

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            break;
        case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            break;
    }
}

// Check for updates and perform if available
void OTAHandler::checkAndUpdate() {
    String latestVersionTag = getLatestVersionTag();

    if (latestVersionTag.length() > 0) {
        // Simple string comparison for version.
        // For more robust version comparison (e.g., v1.0.9 vs v1.0.10),
        // a dedicated version parsing function would be needed.
        // For now, assuming lexicographical comparison is sufficient or versions are simple.
        if (latestVersionTag.compareTo(_currentVersion) > 0) {
            Serial.print("New firmware available: ");
            Serial.print(latestVersionTag);
            Serial.print(" (Current: ");
            Serial.print(_currentVersion);
            Serial.println(")");
            performUpdate(latestVersionTag);
        } else {
            Serial.println("No new firmware available.");
        }
    } else {
        Serial.println("Could not retrieve latest version tag.");
    }
}
