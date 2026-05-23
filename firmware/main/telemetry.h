#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "freertos/event_groups.h"
#include <stdint.h>
#include "freertos/queue.h"

void telemetry_start(EventGroupHandle_t event_group);
void telemetry_set_interval(uint32_t value);
void telemetry_notify_reconnect(void);
QueueHandle_t telemetry_get_queue(void);

#endif