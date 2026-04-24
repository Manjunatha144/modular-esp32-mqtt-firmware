# ESP32 FreeRTOS MQTT Telemetry

## Overview
Embedded firmware project built using ESP-IDF and FreeRTOS.
The ESP32 connects to WiFi, publishes telemetry to an MQTT broker, and subscribes to command messages.

---

## Architecture

ESP32 (ESP-IDF + FreeRTOS)
│
├── NVS Initialization
├── WiFi (Station Mode)
├── MQTT Client
│   ├── Telemetry Publish (5s interval)
│   └── Command Subscribe
│
└── Serial Debug Logging

MQTT Broker: broker.hivemq.com

Subscribers:
- Node-RED Dashboard
- MQTT Explorer

---

## MQTT Topics

Telemetry:
edgepulse/manjunatha144/device_001/telemetry

Command:
edgepulse/manjunatha144/device_001/cmd

---

## Telemetry Payload Example

{
  "device_id": "device_001",
  "seq": 15,
  "uptime_ms": 85745,
  "temp": 27.5
}

- seq increments every publish
- uptime_ms shows device running time
- temp currently simulated value

---

## Project Structure

firmware/        → ESP32 firmware (ESP-IDF)
simulator/       → Python MQTT simulator (Phase 1)
node-red/        → Node-RED flow exports
docs/            → Setup and testing notes
screenshots/     → Dashboard and logs
---

## Key Learning Outcomes

- Practical ESP-IDF firmware development
- WiFi initialization and NVS dependency handling
- MQTT publish/subscribe architecture
- Event-driven programming with ESP event loop
- FreeRTOS-based system design fundamentals
- JSON payload construction in embedded C
- Debugging reboot loops and connection issues