# Smart Plant Monitoring System

ESP32 + FreeRTOS (ESP-IDF) plant monitor that sends notifications via Gmail, WhatsApp, Telegram, and X.

**Course:** BCA152 Microcontrollers, MSU-IIT
**Author:** Candice Guinoo

## Hardware
| Item | Detail |
|---|---|
| Board | ESP32 DevKit V1 (ESP-WROOM-32) |
| Temp/humidity | DHT11 (substitute for DHT22) |
| Soil moisture | Potentiometer (simulated; substitute for capacitive sensor) |

## GPIO mapping
| Signal | GPIO |
|---|---|
| DHT11 DATA (4.7-10 kOhm pull-up to 3.3 V) | 13 |
| Potentiometer wiper (ADC1_CH6) | 34 |

## Build requirements
- VS Code + PlatformIO extension
- Framework: ESP-IDF (`framework = espidf`)
- Versions used: _record platform, ESP-IDF, and toolchain versions here_

## Configuration
1. Copy `config.example` to `src/secrets.h`.
2. Fill in your own credentials. `secrets.h` is git-ignored.

## Build and flash
    pio run
    pio run -t upload
    pio device monitor

## Status
- [x] Phase 0: project setup
- [ ] Phase 1: design docs
- [ ] Phase 2: API access
- [ ] Phase 3: sensors
- [ ] Phase 4: FreeRTOS tasks
- [ ] Phase 5: notifications
- [ ] Phase 6: tests
- [ ] Phase 7: final docs

## Demo reproduction steps
_To be completed._