// OTAHandler.h
#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H

#include <Arduino.h>

class OTAHandler {
public:
    OTAHandler(const char* currentVersion, const char* repoUser, const char* repoName, const char* firmwareFile);
    void checkAndUpdate();

private:
    const char* _currentVersion;
    const char* _repoUser;
    const char* _repoName;
    const char* _versionFile;
    const char* _firmwareFile;
    String _repoApiUrl;

    String getLatestVersionTag();
    void performUpdate(const String& latestVersionTag);
};

#endif // OTA_HANDLER_H
