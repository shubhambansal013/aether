#line 1 "/home/runner/work/aether/aether/OTAHandler.h"
#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H

#include <Arduino.h>

#ifdef UNIT_TEST
#include "OLEDDisplayMock.h"
#include "RGBLEDHandlerMock.h"
#else
#include "OLEDDisplay.h"
#include "RGBLEDHandler.h"
#endif

/**
 * @brief Handles Over-the-Air (OTA) firmware updates for the ESP8266.
 *
 * This class periodically checks a remote JSON manifest for version changes
 * and performs a firmware update using the ESP8266httpUpdate library.
 */
class OTAHandler {
public:
    /**
     * @brief Construct a new OTAHandler object.
     */
    OTAHandler();

    /**
     * @brief Periodically checks for firmware updates.
     *
     * This method should be called in the main loop. It respects the
     * OTA_CHECK_INTERVAL defined in Config.h.
     */
    void handle();

    /**
     * @brief Initializes the OTAHandler with OLED and LED pointers.
     *
     * @param oled Pointer to the OLEDDisplay instance.
     * @param led Pointer to the RGBLEDHandler instance.
     */
    void setup(OLEDDisplay* oled, RGBLEDHandler* led);

private:
    /**
     * @brief Fetches the version manifest from GitHub and compares it with the current version.
     */
    void checkForUpdates();

    /**
     * @brief Downloads and installs the new firmware binary.
     *
     * @param firmwareUrl The direct HTTPS URL to the firmware .bin file.
     */
    void performUpdate(const char* firmwareUrl);

    unsigned long _lastCheck = 0; ///< Timestamp of the last update check.
    OLEDDisplay* _oled = nullptr;
    RGBLEDHandler* _led = nullptr;
};

#endif
