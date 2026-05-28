#include <iostream>
#include <assert.h>
#include "Arduino.h"

// Mock for digitalRead and millis
unsigned long mock_millis = 0;
int mock_pin_state = HIGH;

unsigned long millis() { return mock_millis; }
int digitalRead(int pin) { return mock_pin_state; }
void pinMode(int pin, int mode) {}

#include "../ButtonHandler.h"
#include "../ButtonHandler.cpp"

void test_long_press_logic() {
    mock_millis = 0;
    mock_pin_state = HIGH;
    ButtonHandler button(13);
    button.setup();

    std::cout << "Running test_long_press_logic..." << std::endl;

    // Simulate some time passing
    mock_millis = 5000;

    // Press button
    mock_pin_state = LOW;
    // Debounce wait
    button.isPressed();
    mock_millis += 60;
    button.isPressed();

    // Check if long press triggers before time
    mock_millis = 6000; // Only 1 second passed
    assert(button.isLongPressed() == false);

    // Check if long press triggers after time
    mock_millis = 7100; // 2.1 seconds passed
    assert(button.isLongPressed() == true);

    // Release
    mock_pin_state = HIGH;
    button.isPressed();

    std::cout << "test_long_press_logic passed!" << std::endl;
}

void test_debounce_issue() {
    mock_millis = 0;
    mock_pin_state = HIGH;
    ButtonHandler button(13);
    button.setup();

    std::cout << "Running test_debounce_issue..." << std::endl;

    // 1. Initial press at T=10 (should be ignored by current logic if DEBOUNCE_DELAY=50)
    mock_millis = 10;
    mock_pin_state = LOW;
    button.isPressed();
    // In current logic: currentState=LOW, lastState=HIGH, now-lastDebounce=10-0=10.
    // 10 < 50, so it ignores the press (doesn't set _pressStartTime).
    // BUT it sets _lastState = LOW.

    // 2. Advance time to T=2100
    mock_millis = 2100;
    // Current logic: digitalRead is LOW, _longPressTriggered is false.
    // millis() - _pressStartTime = 2100 - 0 = 2100.
    // 2100 > 2000, so it triggers!
    bool longPressed = button.isLongPressed();

    assert(longPressed == false);

    // 3. Release at T=2200
    mock_pin_state = HIGH;
    mock_millis = 2200;
    button.isPressed(); // Sets _lastDebounceTime = 2200, _lastState = HIGH

    // 4. Press again at T=2210 (within 50ms of release)
    mock_pin_state = LOW;
    mock_millis = 2210;
    button.isPressed(); // Should be ignored because 2210-2200 < 50.

    // 5. Hold until T=5000. It should EVENTUALLY trigger because the state becomes stable.
    mock_millis = 5000;
    button.isPressed(); // This should confirm the LOW state because 5000-2210 > 50
    bool lp = button.isLongPressed();
    assert(lp == true);

    std::cout << "test_debounce_issue finished." << std::endl;
}

int main() {
    test_long_press_logic();
    test_debounce_issue();
    return 0;
}
