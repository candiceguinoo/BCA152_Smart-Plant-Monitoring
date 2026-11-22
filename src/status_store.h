#pragma once
#include <stdbool.h>
#include "app_types.h"

void status_store_init(void);
void status_store_set(const latest_status_t *v);
/* Returns false if nothing has been stored yet or the mutex timed out. */
bool status_store_get(latest_status_t *out);