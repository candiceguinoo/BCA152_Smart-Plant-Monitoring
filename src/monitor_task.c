#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sensor_hal.h"
#include "moisture.h"
#include "validity.h"
#include "state.h"
#include "policy.h"
#include "app_types.h"
#include "status_store.h"
#include "monitor_task.h"

static const char *TAG = "monitor";
static QueueHandle_t s_in;
static QueueHandle_t s_out;

/* PLACEHOLDERS until the calibration step. Document the real values in the report. */
static const moisture_cal_t      CAL = { .raw_dry = 3000, .raw_wet = 1200 };
static const soil_thresholds_t   TH  = { .dry_below_pct = 30, .wet_above_pct = 70, .hysteresis_pct = 5 };

static void monitor_task(void *arg)
{
    (void)arg;
    state_tracker_t tracker;
    cooldown_t cooldown;
    uint32_t count = 0;

    state_tracker_init(&tracker, CONFIRM_READINGS);
    cooldown_init(&cooldown, COOLDOWN_S);

    for (;;) {
        sensor_sample_t s;
        if (xQueueReceive(s_in, &s, pdMS_TO_TICKS(SAMPLE_PERIOD_MS * 3)) != pdTRUE) {
            ESP_LOGW(TAG, "no sensor data for %d ms, sensor_task stalled?", SAMPLE_PERIOD_MS * 3);
            state_tracker_update(&tracker, SOIL_UNKNOWN, false);
            continue;
        }

        int64_t now_s = esp_timer_get_time() / 1000000;
        bool stale = reading_is_stale(s.uptime_s, now_s, SAMPLE_MAX_AGE_S);

        int pct = stale ? -1 : moisture_index_pct(s.raw_adc, &CAL);
        bool soil_valid = pct >= 0;
        bool temp_valid = !stale && s.dht_ok && dht11_temp_is_valid(s.temp_c);
        bool rh_valid   = !stale && s.dht_ok && dht11_rh_is_valid(s.rh_pct);

        if (stale) {
            ESP_LOGW(TAG, "stale sample ignored (age > %d s)", SAMPLE_MAX_AGE_S);
        }
        if (!soil_valid && !stale) {
            ESP_LOGW(TAG, "soil reading INVALID (raw=%d)", s.raw_adc);
        }
        if (!temp_valid || !rh_valid) {
            ESP_LOGW(TAG, "DHT reading INVALID (ok=%d T=%d RH=%d)", (int)s.dht_ok, s.temp_c, s.rh_pct);
        }

        soil_state_t observed = soil_valid ? soil_classify(pct, &TH, tracker.confirmed) : SOIL_UNKNOWN;
        soil_state_t previous = tracker.confirmed;
        bool changed = state_tracker_update(&tracker, observed, soil_valid);

        ESP_LOGI(TAG, "raw=%d moisture=%d%% observed=%s confirmed=%s T=%dC RH=%d%%",
                 s.raw_adc, pct, soil_state_name(observed), soil_state_name(tracker.confirmed),
                 s.temp_c, s.rh_pct);

        latest_status_t latest = {
            .soil_valid = soil_valid, .soil = tracker.confirmed, .moisture_pct = pct,
            .temp_valid = temp_valid, .temp_c = s.temp_c,
            .rh_valid = rh_valid, .rh_pct = s.rh_pct, .uptime_s = now_s
        };
        status_store_set(&latest);

        if (changed) {
            notify_reason_t reason;
            if (policy_reason_for_transition(previous, tracker.confirmed, &reason)) {
                if (policy_should_send(&cooldown, reason, tracker.confirmed, now_s)) {
                    event_msg_t ev = {
                        .reason = reason, .soil = tracker.confirmed, .moisture_pct = pct,
                        .temp_c = s.temp_c, .temp_valid = temp_valid,
                        .rh_pct = s.rh_pct, .rh_valid = rh_valid, .uptime_s = now_s
                    };
                    if (xQueueSend(s_out, &ev, 0) == pdTRUE) {
                        ESP_LOGI(TAG, "event queued: %s (%s -> %s)", notify_reason_label(reason),
                                 soil_state_name(previous), soil_state_name(tracker.confirmed));
                    } else {
                        ESP_LOGE(TAG, "notify_queue full, event dropped");
                    }
                } else {
                    ESP_LOGI(TAG, "%s suppressed by cool-down (%s -> %s)", notify_reason_label(reason),
                             soil_state_name(previous), soil_state_name(tracker.confirmed));
                }
            }
        }

        if (++count % 12u == 0u) {
            ESP_LOGI(TAG, "stack high-water mark: %u bytes free",
                     (unsigned)uxTaskGetStackHighWaterMark(NULL));
        }
    }
}

bool monitor_task_start(QueueHandle_t in, QueueHandle_t out)
{
    s_in = in;
    s_out = out;
    return xTaskCreate(monitor_task, "monitor_task", MONITOR_TASK_STACK, NULL,
                       MONITOR_TASK_PRIO, NULL) == pdPASS;
}