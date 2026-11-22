#include <stddef.h>
#include "moisture.h"

bool adc_raw_is_valid(int raw)
{
    return raw >= ADC_RAW_MIN_VALID && raw <= ADC_RAW_MAX_VALID;
}

bool moisture_cal_is_valid(const moisture_cal_t *cal)
{
    if (cal == NULL) {
        return false;
    }
    if (!adc_raw_is_valid(cal->raw_dry) || !adc_raw_is_valid(cal->raw_wet)) {
        return false;
    }
    int span = cal->raw_wet - cal->raw_dry;
    if (span < 0) {
        span = -span;
    }
    return span >= MOISTURE_MIN_SPAN;
}

int moisture_index_pct(int raw, const moisture_cal_t *cal)
{
    if (!adc_raw_is_valid(raw) || !moisture_cal_is_valid(cal)) {
        return -1;
    }

    long num = (long)(raw - cal->raw_dry) * 100L;
    long den = (long)(cal->raw_wet - cal->raw_dry);
    if (den < 0) {          /* sensor reads higher when drier */
        num = -num;
        den = -den;
    }
    if (num <= 0) {
        return 0;           /* at or beyond the dry reference */
    }
    long pct = (num + den / 2) / den;
    return pct > 100 ? 100 : (int)pct;
}