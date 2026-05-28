#ifndef BUTTONHANDLER_MOCK_H
#define BUTTONHANDLER_MOCK_H

#include "Arduino.h"

class ButtonHandler {
public:
    ButtonHandler(int pin);
    void setup();
    bool isPressed();
    bool isLongPressed();

    bool mock_pressed = false;
    bool mock_longPressed = false;
};

#endif
