/*
 * Smart Plant Monitoring System
 * Board : ESP32 DevKit V1 (ESP-WROOM-32)
 * Stage : Phase 0 - boot check only
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_chip_info.h"

static const char *TAG = "main";

void app_main(void)
{
    esp_chip_info_t info;
    esp_chip_info(&info);

    ESP_LOGI(TAG, "boot ok");
    ESP_LOGI(TAG, "chip cores=%d, revision=%d", info.cores, info.revision);

    /* Heartbeat to confirm the board stays alive. Replaced by tasks later. */
    while (1) {
        ESP_LOGI(TAG, "alive");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}