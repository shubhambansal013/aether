// OTAHandler.h
#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H

#include <Arduino.h>
#include <ArduinoOTA.h> // Include for ArduinoOTA functionality

class OTAHandler {
public:
    // Constructor for GitHub-based OTA
    OTAHandler(const char* currentVersion, const char* repoUser, const char* repoName, const char* firmwareFile);
    
    // Method for GitHub-based OTA update check and perform
    void checkAndUpdate();

    // Method to set up ArduinoOTA (for arduino-cli based updates)
    void setupArduinoOTA();
    
    // Method to handle ArduinoOTA events in the loop
    void handleArduinoOTA();

private:
    // GitHub-based OTA members
    const char* _currentVersion;
    const char* _repoUser;
    const char* _repoName;
    const char* _versionFile; // This variable was not used in the .cpp, can be removed if not needed.
    const char* _firmwareFile;
    String _repoApiUrl;

    String getLatestVersionTag();
    void performUpdate(const String& latestVersionTag);
};

#endif // OTA_HANDLER_H
