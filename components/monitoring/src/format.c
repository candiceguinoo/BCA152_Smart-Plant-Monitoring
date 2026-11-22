#include <stdio.h>
#include "format.h"

static void reason_text(const message_data_t *d, char *out, size_t len)
{
    switch (d->reason) {
    case REASON_INITIAL:
        snprintf(out, len, "initial status after boot");
        break;
    case REASON_ALERT:
        snprintf(out, len, "confirmed change to %s", soil_state_name(d->soil));
        break;
    case REASON_RECOVERY:
        snprintf(out, len, "soil returned to NORMAL");
        break;
    case REASON_DEMO:
        snprintf(out, len, "demo test, cool-down bypassed");
        break;
    default:
        snprintf(out, len, "unknown");
        break;
    }
}

static bool input_ok(char *buf, size_t len, const message_data_t *d)
{
    if (buf == NULL || len == 0) {
        return false;
    }
    buf[0] = '\0';
    return d != NULL && d->plant_id != NULL && d->timestamp != NULL;
}

int format_message(char *buf, size_t len, const message_data_t *d)
{
    if (!input_ok(buf, len, d)) {
        return -1;
    }

    char soil[40];
    char temp[40];
    char rh[40];
    char why[64];

    if (d->moisture_pct >= 0) {
        snprintf(soil, sizeof soil, "%s (%d %%)", soil_state_name(d->soil), d->moisture_pct);
    } else {
        snprintf(soil, sizeof soil, "%s", soil_state_name(d->soil));
    }
    if (d->temp_valid) {
        snprintf(temp, sizeof temp, "%d C", d->temp_c);
    } else {
        snprintf(temp, sizeof temp, "n/a (sensor fault)");
    }
    if (d->rh_valid) {
        snprintf(rh, sizeof rh, "%d %%RH", d->rh_pct);
    } else {
        snprintf(rh, sizeof rh, "n/a (sensor fault)");
    }
    reason_text(d, why, sizeof why);

    int n = snprintf(buf, len, "[%s] %s | Soil: %s | Temp: %s | Humidity: %s | Time: %s | Reason: %s",
                     d->plant_id, notify_reason_label(d->reason), soil, temp, rh, d->timestamp, why);
    if (n < 0 || (size_t)n >= len) {
        buf[len - 1] = '\0';
        return -1;
    }
    return n;
}

int format_subject(char *buf, size_t len, const message_data_t *d)
{
    if (!input_ok(buf, len, d)) {
        return -1;
    }
    int n = snprintf(buf, len, "[%s] %s: soil %s", d->plant_id,
                     notify_reason_label(d->reason), soil_state_name(d->soil));
    if (n < 0 || (size_t)n >= len) {
        buf[len - 1] = '\0';
        return -1;
    }
    return n;
}