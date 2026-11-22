#pragma once
#include <stdbool.h>
#include <stdint.h>

/* One raw acquisition. Validity decisions are made by the monitoring logic,
 * not here, so a faulty value is never silently corrected. */
typedef struct {
    int     raw_adc;     /* 12-bit ADC count from the soil (potentiometer) input */
    bool    dht_ok;      /* false if the DHT read failed (timeout or checksum) */
    int     temp_c;      /* valid only when dht_ok */
    int     rh_pct;      /* valid only when dht_ok */
    int64_t uptime_s;    /* monotonic time of the acquisition */
} sensor_sample_t;

void sensor_hal_init(void);
void sensor_hal_read(sensor_sample_t *out);