#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "telemetry.h"

static const char *TAG = "APP";

/* System Event Group */
static EventGroupHandle_t system_event_group;

/* WiFi bit comes from wifi_manager */
#define WIFI_CONNECTED_BIT BIT0

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Fully Modular Firmware");

    /* Initialize NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* Create Event Group */
    system_event_group = xEventGroupCreate();

    /* Start WiFi */
    wifi_init(system_event_group);

    /* Wait for WiFi connection */
    ESP_LOGI(TAG, "Waiting for WiFi connection...");
    xEventGroupWaitBits(system_event_group,
                        WIFI_CONNECTED_BIT,
                        pdFALSE,
                        pdTRUE,
                        portMAX_DELAY);

    ESP_LOGI(TAG, "WiFi connected. Starting MQTT...");

   telemetry_start(system_event_group);   // Create queue first
   mqtt_start(system_event_group);        // Then start MQTT publish task

    /* Idle loop */
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}