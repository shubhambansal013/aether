#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

class ButtonHandler {
public:
    ButtonHandler(int pin);
    void setup();
    
    // Returns true only ONCE per physical press
    bool isPressed();

private:
    int _pin;
    bool _lastState;
    unsigned long _lastDebounceTime;
    static const unsigned long DEBOUNCE_DELAY = 250; 
};

#endif
