#include "sensor_hal.h"
#include "sensors_config.h"
#include "esp_timer.h"

static int64_t uptime_now_s(void)
{
    return esp_timer_get_time() / 1000000;
}

#if SENSOR_USE_SIM

/* Scripted scenario, one step per sample (5 s). One cycle = 72 samples = 6 min:
 *   0-11  NORMAL      12-23 DRY       24-35 NORMAL (recovery)
 *   36-47 WET         48-59 NORMAL    60-63 ADC fault (raw = 0)
 *   64-67 DHT read failure            68-71 NORMAL
 * Raw values follow the placeholder calibration (dry 3000, wet 1200). */
#define SIM_CYCLE 72u

static uint32_t s_count;

void sensor_hal_init(void)
{
    s_count = 0;
}

void sensor_hal_read(sensor_sample_t *out)
{
    uint32_t n = s_count++ % SIM_CYCLE;
    int base;

    if (n >= 12u && n < 24u) {
        base = 2900;                       /* DRY */
    } else if (n >= 36u && n < 48u) {
        base = 1300;                       /* WET */
    } else {
        base = 2100;                       /* NORMAL */
    }

    int jitter = (int)((n * 37u) % 41u) - 20;
    out->raw_adc = base + jitter;
    if (n >= 60u && n < 64u) {
        out->raw_adc = 0;                  /* wiper disconnected */
    }
    out->dht_ok = !(n >= 64u && n < 68u);
    out->temp_c = 26 + (int)((n / 6u) % 4u);
    out->rh_pct = 55 + (int)(n % 7u);
    out->uptime_s = uptime_now_s();
}

#else
#error "Real sensor backend is added in a later step"
#endif