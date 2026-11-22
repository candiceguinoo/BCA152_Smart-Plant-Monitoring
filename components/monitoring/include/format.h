#pragma once
#include <stdbool.h>
#include <stddef.h>
#include "state.h"
#include "policy.h"

typedef struct {
    const char     *plant_id;       /* e.g. "plant-01" */
    int             temp_c;
    bool            temp_valid;
    int             rh_pct;
    bool            rh_valid;
    soil_state_t    soil;
    int             moisture_pct;   /* 0..100, or -1 when unavailable */
    const char     *timestamp;      /* e.g. "2026-11-22 18:57:00" or "time not synced" */
    notify_reason_t reason;
} message_data_t;

/* Writes the message body. Returns the length, or -1 on bad input or truncation
 * (the buffer is still NUL-terminated when len > 0).
 * Invalid sensor values are printed as n/a, never as numbers. */
int format_message(char *buf, size_t len, const message_data_t *d);

/* Short subject line for email. Same return convention. */
int format_subject(char *buf, size_t len, const message_data_t *d);