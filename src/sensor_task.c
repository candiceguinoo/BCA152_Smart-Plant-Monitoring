#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sensor_hal.h"
#include "app_types.h"
#include "sensor_task.h"

static const char *TAG = "sensor";
static QueueHandle_t s_out;

/* Owns the raw acquisition. Samples are copied into the queue, never shared. */
static void sensor_task(void *arg)
{
    (void)arg;
    TickType_t last_wake = xTaskGetTickCount();
    uint32_t count = 0;

    for (;;) {
        sensor_sample_t s;
        sensor_hal_read(&s);

        if (xQueueSend(s_out, &s, 0) != pdTRUE) {
            ESP_LOGW(TAG, "sensor_queue full, sample dropped");
        }
        if (++count % 12u == 0u) {
            ESP_LOGI(TAG, "stack high-water mark: %u bytes free",
                     (unsigned)uxTaskGetStackHighWaterMark(NULL));
        }

        /* fixed period, independent of how long the read took */
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
    }
}

bool sensor_task_start(QueueHandle_t out)
{
    s_out = out;
    return xTaskCreate(sensor_task, "sensor_task", SENSOR_TASK_STACK, NULL,
                       SENSOR_TASK_PRIO, NULL) == pdPASS;
}