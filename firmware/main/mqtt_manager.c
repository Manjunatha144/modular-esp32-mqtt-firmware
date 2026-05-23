#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "esp_log.h"
#include "mqtt_client.h"

#include "freertos/task.h"
#include "freertos/queue.h"

#include "mqtt_manager.h"
#include "telemetry.h"

#define CMD_TOPIC        "edgepulse/manjunatha144/device_001/cmd"
#define TELEMETRY_TOPIC  "edgepulse/manjunatha144/device_001/telemetry"

static const char *TAG = "MQTT_MANAGER";

static EventGroupHandle_t system_event_group;
static esp_mqtt_client_handle_t global_client = NULL;


/* ---------- MQTT Publish Task (Queue Consumer) ---------- */
static void mqtt_publish_task(void *arg)
{
    QueueHandle_t queue = telemetry_get_queue();
    char payload[256];

    while (1) {

        if (xQueueReceive(queue, payload, portMAX_DELAY) == pdTRUE) {

            if (global_client != NULL) {

                esp_mqtt_client_publish(global_client,
                                        TELEMETRY_TOPIC,
                                        payload,
                                        0, 0, 0);

                ESP_LOGI(TAG, "MQTT Published: %s", payload);
            }
        }
    }
}


/* ---------- MQTT Event Handler ---------- */
static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {

    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        xEventGroupSetBits(system_event_group, MQTT_CONNECTED_BIT);
        esp_mqtt_client_subscribe(global_client, CMD_TOPIC, 0);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        xEventGroupClearBits(system_event_group, MQTT_CONNECTED_BIT);
        telemetry_notify_reconnect();
        break;

    case MQTT_EVENT_DATA: {
        char buffer[128];
        int len = event->data_len < sizeof(buffer) - 1 ? event->data_len : sizeof(buffer) - 1;

        memcpy(buffer, event->data, len);
        buffer[len] = '\0';

        ESP_LOGI(TAG, "CMD received: %s", buffer);

        char *pos = strstr(buffer, "interval_ms");
        if (pos) {
            char *colon = strchr(pos, ':');
            if (colon) {
                uint32_t new_interval = atoi(colon + 1);
                telemetry_set_interval(new_interval);
            }
        }
        break;
    }

    default:
        break;
    }
}


/* ---------- Start MQTT ---------- */
void mqtt_start(EventGroupHandle_t event_group)
{
    system_event_group = event_group;

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://broker.hivemq.com",
    };

    global_client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(global_client,
                                   ESP_EVENT_ANY_ID,
                                   mqtt_event_handler,
                                   NULL);

    esp_mqtt_client_start(global_client);

    /* Start MQTT publish task */
    xTaskCreate(mqtt_publish_task,
                "mqtt_publish_task",
                4096,
                NULL,
                5,
                NULL);
}


/* ---------- Get MQTT Client ---------- */
esp_mqtt_client_handle_t mqtt_get_client(void)
{
    return global_client;
}