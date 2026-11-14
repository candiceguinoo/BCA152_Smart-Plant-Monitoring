# States and Notification Policy

## Soil moisture states
Moisture index = 0-100 % (0 = dry end, 100 = wet end), computed from the potentiometer's calibrated ADC range.
Sensor direction: TBD (document after calibration).

| State | Moisture index | Meaning |
|---|---|---|
| DRY | < TBD % | Plant needs water |
| NORMAL | TBD - TBD % | Healthy range |
| WET | > TBD % | Over-watered |

Thresholds are set from measured dry/wet ADC values (see calibration table, Phase 3).
They are plant-specific and must be adjusted per plant.

## Hysteresis and confirmation
- A state change is accepted only after **3 successive valid readings** in the new state.
- Hysteresis band: TBD % around each threshold to prevent flapping.

## Reading validity (DHT11 limits)
A reading is marked invalid (never reported as normal) if:
- DHT11 checksum error or timeout
- Temperature outside 0-50 C or humidity outside 20-90 %RH
- ADC reading stuck at 0 or 4095 (wiper disconnected)
- Reading older than TBD seconds (stale)

## Notification policy
| Type | Trigger |
|---|---|
| Initial report | Once after boot, Wi-Fi and time sync ready |
| Alert | Confirmed transition into DRY or WET |
| Recovery | Confirmed return to NORMAL |
| Summary | Optional periodic (e.g. every 6 h) |

- Cool-down: at least 15 min for repeated identical alerts (configurable).
- Demo trigger may bypass the cool-down only if labeled "DEMO TEST" in the message.
- Every message contains: plant ID, temperature, humidity, soil status, timestamp, reason.