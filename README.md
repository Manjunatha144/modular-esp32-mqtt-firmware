# Modular ESP32 MQTT Firmware
### ESP-IDF + FreeRTOS Telemetry System

Modular embedded firmware developed using ESP-IDF and FreeRTOS implementing MQTT-based telemetry communication, queue-driven task decoupling, WiFi connectivity, persistent configuration storage, and system health monitoring.

## Key Features

- Modular ESP-IDF firmware architecture
- FreeRTOS task-based design
- Queue-based producer-consumer telemetry pipeline
- MQTT publish/subscribe communication
- WiFi reconnect handling
- EventGroup synchronization
- Persistent configuration using NVS
- Heap and RSSI telemetry monitoring
- Real-time JSON telemetry streaming
## System Architecture

```text
+----------------------+
|   Telemetry Task     |
|----------------------|
| Generate JSON Data   |
| Heap/RSSI Monitoring |
| Reliability Metrics  |
+----------+-----------+
           |
           v
+----------------------+
|   FreeRTOS Queue     |
+----------+-----------+
           |
           v
+----------------------+
|  MQTT Publish Task   |
|----------------------|
| Queue Consumer       |
| MQTT Publish Handler |
+----------+-----------+
           |
           v
+----------------------+
|   MQTT Broker        |
+----------+-----------+
           |
           v
+----------------------+
| Node-RED Dashboard   |
+----------------------+
```
## Firmware Architecture

### app_main.c

Responsible for:
- NVS initialization
- WiFi initialization
- MQTT startup sequencing
- Telemetry startup

Waits for WiFi connection before MQTT initialization.

---

### wifi_manager.c / wifi_manager.h

Responsibilities:
- WiFi station initialization
- WiFi event handling
- Reconnect handling
- EventGroup synchronization

---

### mqtt_manager.c / mqtt_manager.h

Responsibilities:
- MQTT client initialization
- MQTT event callbacks
- MQTT publish task
- Queue consumer handling
- Topic subscription handling

---

### telemetry.c / telemetry.h

Responsibilities:
- Telemetry generation
- Queue producer handling
- JSON payload creation
- Heap monitoring
- RSSI monitoring
- Reliability statistics
- NVS persistent interval storage
