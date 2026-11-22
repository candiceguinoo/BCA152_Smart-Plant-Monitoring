idf_component_register(SRCS "main.c" "sensor_task.c" "monitor_task.c" "notify_task.c" "status_store.c"
                       INCLUDE_DIRS "."
                       REQUIRES monitoring sensors esp_timer)