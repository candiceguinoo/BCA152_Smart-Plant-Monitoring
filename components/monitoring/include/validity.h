#pragma once
#include <stdbool.h>
#include <stdint.h>

/* DHT11 plausibility limits used by this project */
#define DHT11_TEMP_MIN_C   0
#define DHT11_TEMP_MAX_C   50
#define DHT11_RH_MIN_PCT   20
#define DHT11_RH_MAX_PCT   90

bool dht11_temp_is_valid(int temp_c);
bool dht11_rh_is_valid(int rh_pct);

/* A reading is stale if it is older than max_age_s, or time went backwards. */
bool reading_is_stale(int64_t reading_time_s, int64_t now_s, int64_t max_age_s);