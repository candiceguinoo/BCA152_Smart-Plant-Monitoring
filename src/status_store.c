#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "status_store.h"

#define STORE_LOCK_TIMEOUT_MS 100

static SemaphoreHandle_t s_mutex;
static latest_status_t   s_latest;
static bool              s_has_value;

void status_store_init(void)
{
    s_mutex = xSemaphoreCreateMutex();
    s_has_value = false;
}

void status_store_set(const latest_status_t *v)
{
    if (s_mutex == NULL || v == NULL) {
        return;
    }
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(STORE_LOCK_TIMEOUT_MS)) == pdTRUE) {
        s_latest = *v;
        s_has_value = true;
        xSemaphoreGive(s_mutex);
    }
}

bool status_store_get(latest_status_t *out)
{
    bool ok = false;
    if (s_mutex == NULL || out == NULL) {
        return false;
    }
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(STORE_LOCK_TIMEOUT_MS)) == pdTRUE) {
        if (s_has_value) {
            *out = s_latest;
            ok = true;
        }
        xSemaphoreGive(s_mutex);
    }
    return ok;
}