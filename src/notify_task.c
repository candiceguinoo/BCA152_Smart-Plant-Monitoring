#include <stdio.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "format.h"
#include "app_types.h"
#include "status_store.h"
#include "notify_task.h"

static const char *TAG = "notify";
static QueueHandle_t s_in;

/* Wall-clock text once SNTP has set the time, otherwise an uptime fallback. */
static void make_timestamp(char *out, size_t len, int64_t uptime_s)
{
    time_t now = time(NULL);
    if (now > 1700000000) {
        struct tm tm_now;
        localtime_r(&now, &tm_now);
        strftime(out, len, "%Y-%m-%d %H:%M:%S", &tm_now);
    } else {
        snprintf(out, len, "uptime %llds", (long long)uptime_s);
    }
}

static void notify_task(void *arg)
{
    (void)arg;
    event_msg_t ev;

    for (;;) {
        if (xQueueReceive(s_in, &ev, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        char ts[40];
        char subject[80];
        char body[256];
        make_timestamp(ts, sizeof ts, ev.uptime_s);

        message_data_t d = {
            .plant_id = APP_PLANT_ID,
            .temp_c = ev.temp_c, .temp_valid = ev.temp_valid,
            .rh_pct = ev.rh_pct, .rh_valid = ev.rh_valid,
            .soil = ev.soil, .moisture_pct = ev.moisture_pct,
            .timestamp = ts, .reason = ev.reason
        };

        if (format_subject(subject, sizeof subject, &d) < 0 ||
            format_message(body, sizeof body, &d) < 0) {
            ESP_LOGE(TAG, "message formatting failed, event dropped");
            continue;
        }

        /* Send stub. Real channels (Telegram, Gmail, WhatsApp) replace these logs. */
        ESP_LOGI(TAG, "[SEND STUB] subject: %s", subject);
        ESP_LOGI(TAG, "[SEND STUB] body: %s", body);

        latest_status_t latest;
        if (status_store_get(&latest)) {
            ESP_LOGI(TAG, "latest shared status: soil=%s moisture=%d%%",
                     soil_state_name(latest.soil), latest.moisture_pct);
        }
    }
}

bool notify_task_start(QueueHandle_t in)
{
    s_in = in;
    return xTaskCreate(notify_task, "notify_task", NOTIFY_TASK_STACK, NULL,
                       NOTIFY_TASK_PRIO, NULL) == pdPASS;
}