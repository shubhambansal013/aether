#include "ButtonHandler.h"

ButtonHandler::ButtonHandler(int pin) 
    : _pin(pin), _lastState(HIGH), _lastDebounceTime(0) {}

void ButtonHandler::setup() {
    pinMode(_pin, INPUT_PULLUP);
}

bool ButtonHandler::isPressed() {
    bool currentState = digitalRead(_pin);
    bool pressed = false;

    // Check if state changed from HIGH to LOW (Edge Detection)
    if (currentState == LOW && _lastState == HIGH) {
        if ((millis() - _lastDebounceTime) > DEBOUNCE_DELAY) {
            pressed = true;
            _lastDebounceTime = millis();
        }
    }

    _lastState = currentState;
    return pressed;
}
