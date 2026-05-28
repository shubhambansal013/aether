# Agent Instructions for Air Quality Monitor

This document provides context and instructions for AI agents working on this codebase.

## Project Overview
This is an ESP8266-based Air Quality Monitor. It uses a PMSA003 sensor for PM data, a DHT22 for temperature and humidity, an SSD1306 OLED for display, and a WS2812 (NeoPixel) for visual status.

## Architecture
The project follows an object-oriented design:
- `SystemController`: Manages the application state, mode transitions, and coordinates between hardware handlers.
- `aether.ino`: Main entry point, instantiates `SystemController` and hardware handlers.
- `Hardware Handlers`: Individual classes for each component (e.g., `PMSensor`, `OLEDDisplay`, etc.).

## Pin Configuration (NodeMCU)
- PM Sensor: RX=14 (D5), SET=12 (D6)
- OLED Display: SDA=4 (D2), SCL=5 (D1)
- DHT22 Sensor: 2 (D4)
- WS2812 LED: 15 (D8)
- Button: 13 (D7)

## Operating Modes
- **ACTIVE**: Continuous PM sensor readings.
- **PASSIVE**: Wake/Sleep cycle for the PM sensor to prolong its lifespan.

## Stealth Mode
Toggled via long-pressing the button. It turns off the OLED and RGB LED while keeping the background tasks (sensor reading and Blynk transmission) running.

## Testing
Unit tests are located in the `tests/` directory. They use mock headers to allow compilation and execution on a host machine (non-ESP8266).
To run tests:
```bash
g++ -I./ -I./tests/ tests/test_system_controller.cpp SystemController.cpp -o tests/runner && ./tests/runner
```

## Guidelines
- Always prefer refactoring logic into `SystemController` or dedicated handlers rather than adding complexity to `aether.ino`.
- Ensure new features are accompanied by appropriate unit tests in `tests/test_system_controller.cpp`.
- Respect existing pin mappings and timing constants in `Config.h`.
