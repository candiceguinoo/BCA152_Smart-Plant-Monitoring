#pragma once
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/* Receives event_msg_t from 'in' and dispatches notifications.
 * Currently logs the message (send stub); real senders are added later. */
bool notify_task_start(QueueHandle_t in);