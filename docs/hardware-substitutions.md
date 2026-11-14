# Instructor-Approved Hardware Substitutions

| Brief | Used | Reason |
|---|---|---|
| Capacitive soil moisture sensor | Potentiometer on GPIO34 (ADC1) | Sensor not available; pot simulates the analog output |
| DHT22 | DHT11 | Availability |

**Approval:** _paste instructor reply / date here_

## Consequences
- DHT11: integer resolution, +/-2 C, +/-5 %RH, range 0-50 C and 20-80 %RH, min sample interval 1 s (5 s used).
- Dry/wet states are two documented knob positions instead of real soil.