#pragma once
#include <stdbool.h>

typedef enum {
    SOIL_UNKNOWN = 0,
    SOIL_DRY,
    SOIL_NORMAL,
    SOIL_WET
} soil_state_t;

/* Thresholds on the 0-100 % moisture index.
 *   index <  dry_below_pct  -> DRY
 *   index >  wet_above_pct  -> WET
 * hysteresis_pct widens the band needed to LEAVE DRY or WET, so a reading
 * hovering at a threshold does not flip the state back and forth. */
typedef struct {
    int dry_below_pct;
    int wet_above_pct;
    int hysteresis_pct;
} soil_thresholds_t;

bool soil_thresholds_valid(const soil_thresholds_t *t);

/* Classify one reading. 'current' is the last confirmed state (for hysteresis).
 * Returns SOIL_UNKNOWN if pct is outside 0..100. */
soil_state_t soil_classify(int pct, const soil_thresholds_t *t, soil_state_t current);

const char *soil_state_name(soil_state_t s);

/* Confirms a state change only after 'required' successive valid readings. */
typedef struct {
    soil_state_t confirmed;
    soil_state_t candidate;
    int streak;
    int required;
} state_tracker_t;

void state_tracker_init(state_tracker_t *tr, int required);

/* Feed one observation. Invalid readings reset the streak and never confirm
 * anything. Returns true when the confirmed state just changed. */
bool state_tracker_update(state_tracker_t *tr, soil_state_t observed, bool valid);