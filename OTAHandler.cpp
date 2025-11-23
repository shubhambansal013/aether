#include "OTAHandler.h"
#include <ESP8266WiFi.h>

OTAHandler* OTAHandler::_instance = nullptr;

OTAHandler::OTAHandler()
  : _initialized(false), _lastProgress(0), _password(nullptr) {
  _hostname[0] = '\0';
}

void OTAHandler::begin(const char* password, const char* baseHostname) {
    if (_initialized) return;

    _password = password;

    // Build hostname from base + chip id: e.g., airmon-1A2B3C
    uint32_t id = ESP.getChipId();
    if (baseHostname && baseHostname[0] != '\0') {
        snprintf(_hostname, sizeof(_hostname), "%s-%06X", baseHostname, (unsigned int)id);
    } else {
        snprintf(_hostname, sizeof(_hostname), "airmon-%06X", (unsigned int)id);
    }

    ArduinoOTA.setHostname(_hostname);

    if (password && password[0] != '\0') {
        ArduinoOTA.setPassword(password);
    }

    // set instance pointer so callbacks can reference this object
    OTAHandler::_instance = this;

    ArduinoOTA.onStart([]() {
        Serial.println("\nOTA: Start");
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("\nOTA: End");
        Serial.println();
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        if (OTAHandler::_instance) {
            if (total > 0) {
                OTAHandler::_instance->_lastProgress = (uint8_t)((progress * 100) / total);
            } else {
                OTAHandler::_instance->_lastProgress = 0;
            }
        }
        Serial.printf("OTA: Progress: %u%%\r", (unsigned int)(OTAHandler::_instance ? OTAHandler::_instance->_lastProgress : 0));
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("\nOTA Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();
    Serial.print("OTA Ready. Hostname: ");
    Serial.println(_hostname);
    if (_password && _password[0] != '\0') {
        Serial.println("OTA password: (set)");
    } else {
        Serial.println("OTA password: (not set)");
    }

    _initialized = true;
}

void OTAHandler::handle() {
    if (!_initialized) return;
    ArduinoOTA.handle();
}