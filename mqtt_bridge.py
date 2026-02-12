import json
import subprocess
import paho.mqtt.client as mqtt

#Configuration
BROKER = "localhost"              # Broker on same Pi
PORT   = 1883
TOPIC  = "lab/pac"
             


# Start C program
proc = subprocess.Popen(
    ["./main"],
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT,
    text=True,
    bufsize=1
)

# Setup MQTT client
client = mqtt.Client()
client.connect(BROKER, PORT, 60)
client.loop_start()

print("MQTT bridge started...")

try:
    for line in proc.stdout:
        line = line.strip()

        if not line:
            continue

        try:
            payload = json.loads(line)
        except json.JSONDecodeError:
            # Skip non-JSON lines
            continue

        client.publish(TOPIC, json.dumps(payload), qos=0)

except KeyboardInterrupt:
    print("Stopping...")

finally:
    client.loop_stop()
    proc.terminate()
    client.disconnect()
