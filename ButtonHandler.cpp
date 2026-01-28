#include "ButtonHandler.h"
#include "Config.h"

ButtonHandler::ButtonHandler(int pin) 
    : _pin(pin), _lastState(HIGH), _lastDebounceTime(0), _pressStartTime(0), _longPressTriggered(false) {}

void ButtonHandler::setup() {
    pinMode(_pin, INPUT_PULLUP);
}

bool ButtonHandler::isPressed() {
    bool currentState = digitalRead(_pin);
    bool shortPressDetected = false;
    unsigned long now = millis();

    // Detect Press (Falling Edge)
    if (currentState == LOW && _lastState == HIGH) {
        if (now - _lastDebounceTime > DEBOUNCE_DELAY) {
            _pressStartTime = now;
            _longPressTriggered = false;
        }
    }

    // Detect Release (Rising Edge)
    if (currentState == HIGH && _lastState == LOW) {
        // If it wasn't a long press, trigger short press
        if (!_longPressTriggered && (now - _pressStartTime > DEBOUNCE_DELAY)) {
            shortPressDetected = true;
        }
        _lastDebounceTime = now;
    }

    _lastState = currentState;
    return shortPressDetected;
}

bool ButtonHandler::isLongPressed() {
    // If button is held and threshold reached, trigger once
    if (digitalRead(_pin) == LOW && !_longPressTriggered) {
        if (millis() - _pressStartTime > BUTTON_LONG_PRESS_TIME) {
            _longPressTriggered = true;
            return true;
        }
    }
    return false;
}
