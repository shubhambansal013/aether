#line 1 "/home/runner/work/aether/aether/ButtonHandler.cpp"
#include "ButtonHandler.h"
#include "Config.h"

ButtonHandler::ButtonHandler(int pin) 
    : _pin(pin), _lastState(HIGH), _confirmedState(HIGH), _lastDebounceTime(0), _pressStartTime(0), _longPressTriggered(false) {}

void ButtonHandler::setup() {
    pinMode(_pin, INPUT_PULLUP);
}

bool ButtonHandler::isPressed() {
    bool reading = digitalRead(_pin);
    bool shortPressDetected = false;
    unsigned long now = millis();

    if (reading != _lastState) {
        _lastDebounceTime = now;
    }

    if ((now - _lastDebounceTime) > DEBOUNCE_DELAY) {
        // If the state has been stable for longer than DEBOUNCE_DELAY,
        // check if it's different from the confirmed state.
        // We use _pressStartTime == 0 or some other mechanism to know if we are currently "pressed"
        // But let's stick to a confirmed state.

        if (reading != _confirmedState) {
            _confirmedState = reading;

            if (_confirmedState == LOW) {
                // Button Pressed
                _pressStartTime = _lastDebounceTime; // Use the time when the signal first changed
                _longPressTriggered = false;
            } else {
                // Button Released
                if (!_longPressTriggered && (now - _pressStartTime > DEBOUNCE_DELAY)) {
                    shortPressDetected = true;
                }
                _pressStartTime = 0; // Reset press start time on release
            }
        }
    }

    _lastState = reading;
    return shortPressDetected;
}

bool ButtonHandler::isLongPressed() {
    // If button is held (pressStartTime > 0) and threshold reached, trigger once
    if (_pressStartTime > 0 && digitalRead(_pin) == LOW && !_longPressTriggered) {
        if (millis() - _pressStartTime > BUTTON_LONG_PRESS_TIME) {
            _longPressTriggered = true;
            return true;
        }
    }
    return false;
}
