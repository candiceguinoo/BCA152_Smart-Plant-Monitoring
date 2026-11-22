/*
 * Smart Plant Monitoring System
 * Board : ESP32 DevKit V1 (ESP-WROOM-32)
 *
 * sensor_task  (prio 5) -> sensor_queue -> monitor_task (prio 4)
 *                                              |
 *                                        notify_queue -> notify_task (prio 3)
 * status_store (mutex) holds the latest processed values.
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "sensor_hal.h"
#include "app_types.h"
#include "status_store.h"
#include "sensor_task.h"
#include "monitor_task.h"
#include "notify_task.h"

#define SENSOR_QUEUE_LEN  8
#define NOTIFY_QUEUE_LEN  8

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "boot ok");

    status_store_init();
    sensor_hal_init();

    QueueHandle_t sensor_q = xQueueCreate(SENSOR_QUEUE_LEN, sizeof(sensor_sample_t));
    QueueHandle_t notify_q = xQueueCreate(NOTIFY_QUEUE_LEN, sizeof(event_msg_t));
    if (sensor_q == NULL || notify_q == NULL) {
        ESP_LOGE(TAG, "queue creation failed");
        return;
    }

    /* consumers first, producer last */
    if (!notify_task_start(notify_q) ||
        !monitor_task_start(sensor_q, notify_q) ||
        !sensor_task_start(sensor_q)) {
        ESP_LOGE(TAG, "task creation failed");
        return;
    }
    ESP_LOGI(TAG, "tasks started");
}