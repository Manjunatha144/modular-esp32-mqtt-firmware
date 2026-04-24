\# Test Matrix – ESP32 MQTT Telemetry



\## Test 1 – WiFi Connection

Result: PASS  

ESP32 connects to MANJU\_WIFI and obtains IP address.



\## Test 2 – MQTT Connection

Result: PASS  

MQTT\_EVENT\_CONNECTED observed in serial monitor.



\## Test 3 – Telemetry Publish

Result: PASS  

Telemetry published every 5 seconds.



\## Test 4 – Command Subscription

Result: PASS  

ESP32 receives JSON command on cmd topic.



\## Test 5 – Broker Disconnect Simulation

Result: PASS  

MQTT\_EVENT\_DISCONNECTED observed when network is disabled.



\## Test 6 – Device Reboot

Result: PASS  

Sequence resets to 0 and device reconnects successfully.

