#pragma once
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/* Periodic acquisition. Pushes sensor_sample_t copies into 'out'. */
bool sensor_task_start(QueueHandle_t out);