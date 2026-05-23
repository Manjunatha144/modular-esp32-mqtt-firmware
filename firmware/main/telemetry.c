#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "esp_wifi.h"
#include "esp_system.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "nvs_flash.h"
#include "nvs.h"

#include "telemetry.h"

#define WIFI_CONNECTED_BIT BIT0
#define MQTT_CONNECTED_BIT BIT1

#define MIN_INTERVAL_MS 500
#define MAX_INTERVAL_MS 60000

#define PAYLOAD_MAX_LEN 256
#define HEAP_ALERT_THRESHOLD 150000   // 150 KB

static const char *TAG = "TELEMETRY";

static EventGroupHandle_t system_event_group;
static QueueHandle_t telemetry_queue = NULL;

/* Persistent interval */
static volatile uint32_t interval_ms = 5000;

/* Counters */
static uint32_t sent_count = 0;
static uint32_t failed_count = 0;
static uint32_t reconnect_count = 0;


/* ---------- Load Interval From NVS ---------- */
static void load_interval_from_nvs()
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err == ESP_OK) {
        uint32_t saved_interval;
        err = nvs_get_u32(nvs_handle, "interval", &saved_interval);
        if (err == ESP_OK) {
            interval_ms = saved_interval;
            ESP_LOGI(TAG, "Loaded interval from NVS: %lu", interval_ms);
        }
        nvs_close(nvs_handle);
    }
}

/* ---------- Save Interval To NVS ---------- */
static void save_interval_to_nvs(uint32_t value)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err == ESP_OK) {
        nvs_set_u32(nvs_handle, "interval", value);
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
        ESP_LOGI(TAG, "Saved interval to NVS: %lu", value);
    }
}

/* ---------- Public Setter ---------- */
void telemetry_set_interval(uint32_t value)
{
    if (value >= MIN_INTERVAL_MS && value <= MAX_INTERVAL_MS) {
        interval_ms = value;
        save_interval_to_nvs(value);
        ESP_LOGI(TAG, "Interval updated to %lu ms", interval_ms);
    } else {
        ESP_LOGW(TAG, "Invalid interval received: %lu", value);
    }
}

/* ---------- Reconnect Notifier ---------- */
void telemetry_notify_reconnect(void)
{
    reconnect_count++;
    ESP_LOGI(TAG, "Reconnect count incremented: %lu", reconnect_count);
}

/* ---------- Telemetry Task ---------- */
static void telemetry_task(void *arg)
{
    uint32_t seq = 0;
    char payload[PAYLOAD_MAX_LEN];

    while (1) {

        xEventGroupWaitBits(system_event_group,
                            WIFI_CONNECTED_BIT | MQTT_CONNECTED_BIT,
                            pdFALSE,
                            pdTRUE,
                            portMAX_DELAY);

        seq++;

        uint32_t uptime_ms = esp_timer_get_time() / 1000;
uint32_t heap_free = esp_get_free_heap_size();
uint8_t heap_alert = 0;

if (heap_free < HEAP_ALERT_THRESHOLD) {
    heap_alert = 1;
    ESP_LOGW(TAG, "Heap below threshold! Current: %lu", heap_free);
}

wifi_ap_record_t ap_info;
int rssi = 0;
if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
    rssi = ap_info.rssi;
}

      snprintf(payload, sizeof(payload),
         "{\"device_id\":\"device_001\","
         "\"seq\":%lu,"
         "\"uptime_ms\":%lu,"
         "\"temp\":27.5,"
         "\"heap_free\":%lu,"
         "\"heap_alert\":%u,"
         "\"rssi\":%d,"
         "\"sent\":%lu,"
         "\"failed\":%lu,"
         "\"reconnect\":%lu}",
         seq,
         uptime_ms,
         heap_free,
         heap_alert,
         rssi,
         sent_count,
         failed_count,
         reconnect_count);
        /* Push payload to queue instead of direct publish */
        if (xQueueSend(telemetry_queue, payload, pdMS_TO_TICKS(100)) == pdTRUE) {
            sent_count++;
        } else {
            failed_count++;
            ESP_LOGW(TAG, "Queue full, payload dropped");
        }

        ESP_LOGI(TAG, "Telemetry queued: %s", payload);

        vTaskDelay(pdMS_TO_TICKS(interval_ms));
    }
}

/* ---------- Start Telemetry ---------- */
void telemetry_start(EventGroupHandle_t event_group)
{
    system_event_group = event_group;

    load_interval_from_nvs();

    /* Create queue for 5 payloads */
    telemetry_queue = xQueueCreate(5, PAYLOAD_MAX_LEN);

    xTaskCreate(telemetry_task,
                "telemetry_task",
                4096,
                NULL,
                5,
                NULL);
}

/* ---------- Getter for MQTT module ---------- */
QueueHandle_t telemetry_get_queue(void)
{
    return telemetry_queue;
}