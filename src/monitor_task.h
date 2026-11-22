#pragma once
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/* Reads samples from 'in', validates and classifies them, and posts
 * notification events to 'out'. */
bool monitor_task_start(QueueHandle_t in, QueueHandle_t out);