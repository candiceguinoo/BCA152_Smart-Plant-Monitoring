# Test Plan

| ID | Test | Method | Pass criteria | Result |
|---|---|---|---|---|
| T1 | Startup and sensing | Reset board, watch serial log | Boots cleanly; both sensors read periodically with units and timestamps | |
| T2 | Calibration, dry/wet | Turn pot to dry and wet positions | Expected state shown for each | |
| T3 | Invalid sensor reading | Unplug DHT11 DATA; disconnect pot wiper | Reading flagged invalid, no "normal" report | |
| T4 | Notification policy | Force confirmed transition; repeat | Alert sent once; duplicates suppressed in cool-down | |
| T5 | Loss of connectivity | Turn off router, then restore | Local monitoring continues; reconnects; outcome reported | |
| T6 | Service error / rate limit | Use a wrong token; trigger 429 if possible | Error logged; limited retries; no request loop | |
| T7 | Four-channel delivery | Trigger one event | Gmail, WhatsApp, Telegram, X each receive matching data | |
| T8 | Restart and recovery | Power cycle several times | No crash or hang; no secrets in logs | |

## Unit tests (host-native)
Moisture conversion, status classification, hysteresis/confirmation, cool-down, message formatting.

## Static analysis
Tool, version, findings, and fixes recorded in docs/results/.