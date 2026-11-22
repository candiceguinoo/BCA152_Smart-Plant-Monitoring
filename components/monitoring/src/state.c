#include <stddef.h>
#include "state.h"

bool soil_thresholds_valid(const soil_thresholds_t *t)
{
    if (t == NULL) {
        return false;
    }
    if (t->dry_below_pct < 0 || t->wet_above_pct > 100 || t->hysteresis_pct < 0) {
        return false;
    }
    if (t->dry_below_pct >= t->wet_above_pct) {
        return false;
    }
    /* the DRY exit point must stay below the WET exit point */
    return (t->dry_below_pct + t->hysteresis_pct) < (t->wet_above_pct - t->hysteresis_pct);
}

soil_state_t soil_classify(int pct, const soil_thresholds_t *t, soil_state_t current)
{
    if (pct < 0 || pct > 100 || !soil_thresholds_valid(t)) {
        return SOIL_UNKNOWN;
    }

    int dry_exit = t->dry_below_pct + t->hysteresis_pct;
    int wet_exit = t->wet_above_pct - t->hysteresis_pct;

    if (current == SOIL_DRY && pct < dry_exit) {
        return SOIL_DRY;
    }
    if (current == SOIL_WET && pct > wet_exit) {
        return SOIL_WET;
    }
    if (pct < t->dry_below_pct) {
        return SOIL_DRY;
    }
    if (pct > t->wet_above_pct) {
        return SOIL_WET;
    }
    return SOIL_NORMAL;
}

const char *soil_state_name(soil_state_t s)
{
    switch (s) {
    case SOIL_DRY:    return "DRY";
    case SOIL_NORMAL: return "NORMAL";
    case SOIL_WET:    return "WET";
    default:          return "UNKNOWN";
    }
}

void state_tracker_init(state_tracker_t *tr, int required)
{
    tr->confirmed = SOIL_UNKNOWN;
    tr->candidate = SOIL_UNKNOWN;
    tr->streak = 0;
    tr->required = required < 1 ? 1 : required;
}

bool state_tracker_update(state_tracker_t *tr, soil_state_t observed, bool valid)
{
    if (!valid || observed == SOIL_UNKNOWN) {
        tr->candidate = SOIL_UNKNOWN;
        tr->streak = 0;
        return false;
    }
    if (observed == tr->confirmed) {
        tr->candidate = SOIL_UNKNOWN;
        tr->streak = 0;
        return false;
    }
    if (observed == tr->candidate) {
        tr->streak++;
    } else {
        tr->candidate = observed;
        tr->streak = 1;
    }
    if (tr->streak >= tr->required) {
        tr->confirmed = observed;
        tr->candidate = SOIL_UNKNOWN;
        tr->streak = 0;
        return true;
    }
    return false;
}