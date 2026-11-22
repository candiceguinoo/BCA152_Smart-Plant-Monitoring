#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "policy.h"
#include "state.h"

#define APP_PLANT_ID        "plant-01"

#define SAMPLE_PERIOD_MS    5000
#define SAMPLE_MAX_AGE_S    15
#define CONFIRM_READINGS    3
#define COOLDOWN_S          900

/* Task priorities and stacks (bytes). Higher number = higher priority.
 * Sensing outranks processing, which outranks network work. */
#define SENSOR_TASK_PRIO    5
#define SENSOR_TASK_STACK   4096
#define MONITOR_TASK_PRIO   4
#define MONITOR_TASK_STACK  4096
#define NOTIFY_TASK_PRIO    3
#define NOTIFY_TASK_STACK   8192

/* monitor_task -> notify_task, passed by value through notify_queue */
typedef struct {
    notify_reason_t reason;
    soil_state_t    soil;
    int             moisture_pct;
    int             temp_c;
    bool            temp_valid;
    int             rh_pct;
    bool            rh_valid;
    int64_t         uptime_s;
} event_msg_t;

/* Latest processed values, shared between tasks behind a mutex */
typedef struct {
    bool         soil_valid;
    soil_state_t soil;
    int          moisture_pct;
    bool         temp_valid;
    int          temp_c;
    bool         rh_valid;
    int          rh_pct;
    int64_t      uptime_s;
} latest_status_t;