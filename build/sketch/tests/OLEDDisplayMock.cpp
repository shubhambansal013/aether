#line 1 "/home/runner/work/aether/aether/tests/OLEDDisplayMock.cpp"
#include "OLEDDisplayMock.h"

OLEDDisplay::OLEDDisplay(int sda, int scl) {}
void OLEDDisplay::setup() {}
void OLEDDisplay::update(const SystemData& d) { updateCount++; lastData = d; }
void OLEDDisplay::clear() { clearCount++; }
void OLEDDisplay::printMessage(String l1, String l2) {}
