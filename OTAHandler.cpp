#include "OTAHandler.h"
#include "Config.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <memory>

OTAHandler::OTAHandler() : _lastCheck(0), _oled(nullptr), _led(nullptr) {}

void OTAHandler::setup(OLEDDisplay* oled, RGBLEDHandler* led) {
    _oled = oled;
    _led = led;
}

void OTAHandler::handle() {
    if (WiFi.status() != WL_CONNECTED) {
        // Log periodically even if not connected to show it's alive
        static unsigned long lastWiFiLog = 0;
        if (millis() - lastWiFiLog > 30000) {
            Serial.println(F("OTA: WiFi not connected, skipping check."));
            lastWiFiLog = millis();
        }
        return;
    }

    unsigned long now = millis();
    if (_lastCheck == 0 || (now - _lastCheck >= OTA_CHECK_INTERVAL)) {
        _lastCheck = now;
        checkForUpdates();
    }
}

void OTAHandler::checkForUpdates() {
    Serial.println(F("Checking for updates..."));

    String firmwareUrl = "";

    {
        std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
        client->setInsecure();
        client->setBufferSizes(1024, 1024);

        HTTPClient https;
        if (https.begin(*client, OTA_MANIFEST_URL)) {
            https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
            int httpCode = https.GET();
            if (httpCode == HTTP_CODE_OK) {
                String payload = https.getString();

                StaticJsonDocument<256> doc;
                DeserializationError error = deserializeJson(doc, payload);

                if (!error) {
                    const char* newVersion = doc["version"];
                    const char* url = doc["url"];

                    if (newVersion && strcmp(newVersion, FIRMWARE_VERSION) != 0) {
                        Serial.print(F("New version available: "));
                        Serial.println(newVersion);

                        if (url) {
                            firmwareUrl = String(url);
                        } else {
                            Serial.println(F("Error: Firmware URL missing in manifest."));
                        }
                    } else {
                        Serial.println(F("Firmware is up to date."));
                    }
                } else {
                    Serial.print(F("JSON parsing failed: "));
                    Serial.println(error.f_str());
                }
            } else {
                Serial.print(F("HTTPS GET failed, error: "));
                Serial.println(https.errorToString(httpCode).c_str());
            }
            https.end();
        } else {
            Serial.println(F("Unable to connect to manifest URL"));
        }
    } // client and https go out of scope here, freeing memory

    if (firmwareUrl.length() > 0) {
        performUpdate(firmwareUrl.c_str());
    }
}

void OTAHandler::performUpdate(const char* firmwareUrl) {
    String url = String(firmwareUrl);

    Serial.print(F("Starting OTA update from: "));
    Serial.println(url);
    Serial.print(F("Free heap: "));
    Serial.println(ESP.getFreeHeap());

    if (_oled) _oled->printMessage("Updating...", "Downloading binary");
    if (_led) _led->setColor(0x0000FF); // Blue for update

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    client->setBufferSizes(1024, 1024);

    // The update process is blocking and will reboot on success
    ESPhttpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    t_httpUpdate_return ret = ESPhttpUpdate.update(*client, url);

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("OTA update failed! Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println(F("No updates available."));
            break;
        case HTTP_UPDATE_OK:
            Serial.println(F("OTA update successful! Rebooting..."));
            if (_oled) _oled->printMessage("Update OK", "Rebooting...");
            break;
    }
}
