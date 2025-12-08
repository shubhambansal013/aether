// ResetHandler.h

#ifndef RESET_HANDLER_H
#define RESET_HANDLER_H

#include <Arduino.h>
#include <EEPROM.h>

// Forward declaration to avoid circular dependency, if needed later.
class WiFiHandler;

class ResetHandler {
public:
    /**
     * @brief Constructor. Initializes EEPROM size.
     * @param wifiHandler Reference to the WiFiHandler object for resetting credentials.
     */
    ResetHandler(WiFiHandler& wifiHandler);

    /**
     * @brief Reads EEPROM data, detects rapid power cycles, and triggers a full reset if the threshold is met.
     */
    void checkPowerCycles();

private:
    WiFiHandler& _wifiHandler; // Reference to the WiFiHandler object

    // EEPROM addresses (must not conflict with other EEPROM usage)
    const int BOOT_COUNT_ADDR = 0;
    const int LAST_BOOT_TIME_ADDR = 1;
    const int EEPROM_SIZE = 512;

    // Reset Logic Constants
    const int MAX_BOOT_CYCLES = 7;
    const unsigned long MAX_TIME_MS = 5000;
    const int RESET_DELAY_MS = 500;
};

#endif
