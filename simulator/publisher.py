import json, time, random
import paho.mqtt.client as mqtt

BROKER = "broker.hivemq.com"
PORT = 1883
TOPIC_TLM = "edgepulse/manjunatha144/device_001/telemetry"
TOPIC_CMD = "edgepulse/manjunatha144/device_001/cmd"

interval_ms = 2000
seq = 0

def make_payload():
    global seq
    seq += 1
    return {
        "device_id": "device_001",
        "seq": seq,
        "uptime_ms": int(time.time() * 1000),
        "temp": round(25 + random.random() * 5, 2),
    }

def on_connect(client, userdata, flags, rc, properties=None):
    print("MQTT connected rc=", rc)
    client.subscribe(TOPIC_CMD)

def on_message(client, userdata, msg):
    global interval_ms
    try:
        data = json.loads(msg.payload.decode("utf-8"))
        new_int = int(data.get("interval_ms", interval_ms))
        interval_ms = max(200, min(10000, new_int))
        print("CMD interval_ms =", interval_ms)
    except Exception as e:
        print("CMD parse error:", e)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

while True:
    try:
        client.connect(BROKER, PORT, keepalive=60)
        client.loop_start()
        while True:
            payload = make_payload()
            client.publish(TOPIC_TLM, json.dumps(payload))
            print("PUB", payload)
            time.sleep(interval_ms/1000)
    except Exception as e:
        print("MQTT error:", e, "retry in 3s")
        try:
            client.loop_stop()
        except:
            pass
        time.sleep(3)