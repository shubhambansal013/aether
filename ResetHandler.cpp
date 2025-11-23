// ResetHandler.cpp

#include "ResetHandler.h"
#include "WiFiHandler.h" // Include the WiFiHandler definition

ResetHandler::ResetHandler(WiFiHandler& wifiHandler) 
    : _wifiHandler(wifiHandler) {
    EEPROM.begin(EEPROM_SIZE);
}

void ResetHandler::checkPowerCycles() {
    byte bootCount;
    unsigned long lastBootTime;

    // 1. Read stored values
    EEPROM.get(BOOT_COUNT_ADDR, bootCount);
    EEPROM.get(LAST_BOOT_TIME_ADDR, lastBootTime);
    
    // Safety check for corrupted EEPROM (or first boot)
    if (bootCount > MAX_BOOT_CYCLES || bootCount == 255) { 
        bootCount = 0; 
    }
    
    unsigned long currentTime = millis();
    bool rapidBoot = false;

    // 2. Logic to determine if a rapid boot occurred
    // Compare current boot time with the last one. If they are close, it's a rapid boot.
    if (lastBootTime != 0 && (currentTime - lastBootTime) < MAX_TIME_MS) {
        rapidBoot = true;
        bootCount++;
        Serial.print("Rapid boot detected! Count: ");
        Serial.println(bootCount);
    } else {
        // Not a rapid boot, reset the count for a normal power cycle
        bootCount = 1; 
    }

    // 3. Check for reset trigger
    if (bootCount >= MAX_BOOT_CYCLES) {
        Serial.println("!!! POWER CYCLE RESET TRIGGERED !!!");
        
        // Use the method from the injected WiFiHandler object
        _wifiHandler.resetSettings();

        // Clear the EEPROM boot tracking data
        EEPROM.put(BOOT_COUNT_ADDR, (byte)0);
        EEPROM.put(LAST_BOOT_TIME_ADDR, (unsigned long)0);
        EEPROM.commit();
        
        Serial.println("Restarting into configuration mode...");
        delay(RESET_DELAY_MS);
        ESP.reset();
        delay(5000); // Safety delay (should not be reached)
    }

    // 4. Save current boot data for the next check
    EEPROM.put(BOOT_COUNT_ADDR, bootCount);
    EEPROM.put(LAST_BOOT_TIME_ADDR, currentTime);
    EEPROM.commit();
    Serial.print("Current Boot Count: ");
    Serial.println(bootCount);
}