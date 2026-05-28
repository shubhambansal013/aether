#ifndef BUTTONHANDLER_H
#define BUTTONHANDLER_H

#include <Arduino.h>

class ButtonHandler {
public:
    ButtonHandler(int pin);
    void setup();
    bool isPressed();       // Returns true on short release
    bool isLongPressed();   // Returns true immediately when time threshold met

private:
    int _pin;
    bool _lastState;
    bool _confirmedState;
    unsigned long _lastDebounceTime;
    unsigned long _pressStartTime;
    bool _longPressTriggered;
};

#endif
