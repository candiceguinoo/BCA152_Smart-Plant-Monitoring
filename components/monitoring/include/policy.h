#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "state.h"

#define COOLDOWN_MIN_S       900   /* brief: at least 15 minutes between identical alerts */
#define COOLDOWN_STATE_SLOTS 4     /* one slot per soil_state_t value */

typedef enum {
    REASON_INITIAL = 0,   /* first confirmed status after boot */
    REASON_ALERT,         /* confirmed change into DRY or WET */
    REASON_RECOVERY,      /* confirmed return to NORMAL from DRY or WET */
    REASON_DEMO           /* labeled demonstration trigger, bypasses cool-down */
} notify_reason_t;

/* Decide whether a confirmed state change deserves a notification and why.
 * Returns false when nothing should be sent. */
bool policy_reason_for_transition(soil_state_t prev, soil_state_t now, notify_reason_t *out);

const char *notify_reason_label(notify_reason_t r);

typedef struct {
    int64_t cooldown_s;
    int64_t last_sent_s[COOLDOWN_STATE_SLOTS];
    bool    has_sent[COOLDOWN_STATE_SLOTS];
} cooldown_t;

/* cooldown_s below COOLDOWN_MIN_S is raised to COOLDOWN_MIN_S. */
void cooldown_init(cooldown_t *c, int64_t cooldown_s);

/* ALERT and RECOVERY are limited to one per state per cool-down period.
 * INITIAL and DEMO always pass and do not start a cool-down.
 * Records the send time when it returns true for ALERT or RECOVERY. */
bool policy_should_send(cooldown_t *c, notify_reason_t reason, soil_state_t state, int64_t now_s);