#include <stddef.h>
#include "policy.h"

bool policy_reason_for_transition(soil_state_t prev, soil_state_t now, notify_reason_t *out)
{
    if (out == NULL || now == SOIL_UNKNOWN || prev == now) {
        return false;
    }
    if (prev == SOIL_UNKNOWN) {
        *out = REASON_INITIAL;
    } else if (now == SOIL_DRY || now == SOIL_WET) {
        *out = REASON_ALERT;
    } else {
        *out = REASON_RECOVERY;
    }
    return true;
}

const char *notify_reason_label(notify_reason_t r)
{
    switch (r) {
    case REASON_INITIAL:  return "STATUS REPORT";
    case REASON_ALERT:    return "ALERT";
    case REASON_RECOVERY: return "RECOVERY";
    case REASON_DEMO:     return "DEMO TEST";
    default:              return "UNKNOWN";
    }
}

void cooldown_init(cooldown_t *c, int64_t cooldown_s)
{
    c->cooldown_s = cooldown_s < COOLDOWN_MIN_S ? COOLDOWN_MIN_S : cooldown_s;
    for (int i = 0; i < COOLDOWN_STATE_SLOTS; i++) {
        c->last_sent_s[i] = 0;
        c->has_sent[i] = false;
    }
}

bool policy_should_send(cooldown_t *c, notify_reason_t reason, soil_state_t state, int64_t now_s)
{
    if (reason == REASON_INITIAL || reason == REASON_DEMO) {
        return true;
    }
    if (reason != REASON_ALERT && reason != REASON_RECOVERY) {
        return false;
    }
    if (state <= SOIL_UNKNOWN || (int)state >= COOLDOWN_STATE_SLOTS) {
        return false;
    }

    int slot = (int)state;
    if (c->has_sent[slot]) {
        int64_t elapsed = now_s - c->last_sent_s[slot];
        /* elapsed < 0 means the clock moved backwards: do not block forever */
        if (elapsed >= 0 && elapsed < c->cooldown_s) {
            return false;
        }
    }
    c->has_sent[slot] = true;
    c->last_sent_s[slot] = now_s;
    return true;
}