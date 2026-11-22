#include "validity.h"

bool dht11_temp_is_valid(int temp_c)
{
    return temp_c >= DHT11_TEMP_MIN_C && temp_c <= DHT11_TEMP_MAX_C;
}

bool dht11_rh_is_valid(int rh_pct)
{
    return rh_pct >= DHT11_RH_MIN_PCT && rh_pct <= DHT11_RH_MAX_PCT;
}

bool reading_is_stale(int64_t reading_time_s, int64_t now_s, int64_t max_age_s)
{
    int64_t age = now_s - reading_time_s;
    return age < 0 || age > max_age_s;
}