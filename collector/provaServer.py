# Import package
import paho.mqtt.client as mqtt
import argparse
import json
from database import Database
import tabulate

# Define Variables
MQTT_HOST = "localhost"
MQTT_PORT = 1883
MQTT_KEEPALIVE_INTERVAL = 45
MQTT_TOPIC = "info"
MQTT_MSG = "hello MQTT"

# Define on connect event function
# We shall subscribe to our Topic in this function
def on_connect(self, mosq, obj, rc):
    mqttc.subscribe(MQTT_TOPIC, 0)

# Define on_message event function.
# This function will be invoked every time,
# a new message arrives for the subscribed topic
def on_message(self,mosq, obj, msg):
    print ("Topic: " + str(msg.topic))
    print ("QoS: " + str(msg.qos))
    print ("Payload: " + str(msg.payload))
    receivedData = json.loads(msg.payload)
    temp = receivedData["temp"]
    humidity = receivedData["humidity"]
    light = receivedData["light"]
    gas = receivedData["gas"]
    if receivedData["light"] == 0:
        light = "LOUMINOUS"
    elif receivedData["light"] == 1:
        light = "RELAX"
    else:
        light = "DARK"
    # faccio la connessione a sql e metto i dati, nella nostra app posso anche evitare --> posso usare un df
    with self.connection.cursor() as cursor:
        # Create a new record
        sql = "INSERT INTO `mqttsensors` (`temperature`, `humidity`,`light` ,`gas`) VALUES (%s, %s, %s , %s)"
        cursor.execute(sql, (temp, humidity, light , gas))

    # Commit changes
    self.connection.commit()

    with self.connection.cursor() as cursor2:
        sql = "SELECT * FROM `mqttsensors`"
        cursor2.execute(sql)
        results = cursor2.fetchall()
        header = results[0].keys()
        rows = [x.values() for x in results]
        print(tabulate.tabulate(rows,header,tablefmt='grid'))


# Initiate MQTT Client
mqttc = mqtt.Client()

# Assign event callbacks
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.db = Database()
mqttc.connection = mqttc.db.connect_dbs()

# Connect with MQTT Broker
mqttc.connect(MQTT_HOST, MQTT_PORT, MQTT_KEEPALIVE_INTERVAL)

# Continue monitoring the incoming messages for subscribed topic
mqttc.loop_forever()