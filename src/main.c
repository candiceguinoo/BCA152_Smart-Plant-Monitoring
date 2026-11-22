/*
 * Smart Plant Monitoring System
 * Board : ESP32 DevKit V1 (ESP-WROOM-32)
 * Stage : monitoring component linked; tasks come in Phase 4
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "moisture.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "boot ok");

    /* Sample calibration, replaced by real values after calibration. */
    moisture_cal_t cal = { .raw_dry = 3000, .raw_wet = 1200 };
    ESP_LOGI(TAG, "demo: raw 2100 -> moisture %d %%", moisture_index_pct(2100, &cal));

    while (1) {
        ESP_LOGI(TAG, "alive");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}