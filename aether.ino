#include <Arduino.h>
#include <EEPROM.h>
#include "Config.h"
#include "SystemController.h"

// --- Global Instances ---
PMSensor pmSensor(PM_SENSOR_RX_PIN, PM_SENSOR_SET_PIN);
DHTSensor dhtSensor(DHT_PIN);
OLEDDisplay oled(OLED_SDA_PIN, OLED_SCL_PIN);
RGBLEDHandler led(WS2812_PIN); 
WiFiHandler wifi;
BlynkHandler blynk;
ButtonHandler button(BUTTON_PIN);

SystemController controller(pmSensor, dhtSensor, oled, led, wifi, blynk, button);

void setup() {
    Serial.begin(115200);
    EEPROM.begin(512);
    
    controller.setup();
    
    Serial.println(F("--- System Initialized ---"));
}

void loop() {
    controller.update();
    delay(20); 
}
