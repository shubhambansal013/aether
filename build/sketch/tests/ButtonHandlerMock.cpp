#line 1 "/home/runner/work/aether/aether/tests/ButtonHandlerMock.cpp"
#include "ButtonHandlerMock.h"

ButtonHandler::ButtonHandler(int p) {}
void ButtonHandler::setup() {}
bool ButtonHandler::isPressed() { return mock_pressed; }
bool ButtonHandler::isLongPressed() { return mock_longPressed; }
