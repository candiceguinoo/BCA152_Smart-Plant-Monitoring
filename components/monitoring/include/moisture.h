#pragma once
#include <stdbool.h>

/* ESP32 12-bit ADC. Readings stuck at the rails mean a wiring fault. */
#define ADC_RAW_MIN_VALID   1
#define ADC_RAW_MAX_VALID   4094
#define MOISTURE_MIN_SPAN   200   /* min distance between dry and wet raw values */

/* Calibration: raw ADC value measured at the dry and wet reference points.
 * Works for either sensor direction (raw_dry > raw_wet or raw_dry < raw_wet). */
typedef struct {
    int raw_dry;
    int raw_wet;
} moisture_cal_t;

bool adc_raw_is_valid(int raw);
bool moisture_cal_is_valid(const moisture_cal_t *cal);

/* Returns moisture index 0 (dry) .. 100 (wet), or -1 if raw or cal is invalid. */
int moisture_index_pct(int raw, const moisture_cal_t *cal);