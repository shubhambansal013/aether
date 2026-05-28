#include "RGBLEDHandlerMock.h"

RGBLEDHandler::RGBLEDHandler(int p) : _strip(1, p, 0) {}
void RGBLEDHandler::setup() {}
void RGBLEDHandler::updateLED(float pm) { updateCount++; lastPM = pm; }
void RGBLEDHandler::turnOff() { turnOffCount++; }
void RGBLEDHandler::setColor(uint32_t hex) {}
void RGBLEDHandler::startupSequence() {}
