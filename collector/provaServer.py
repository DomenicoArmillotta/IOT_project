# Import package
import paho.mqtt.client as mqtt
from coapthon.server.coap import CoAP
from server import *
import argparse
import threading
import json
from database import Database
import tabulate
import time
import datetime
import logging
from coap_collector import Motion
from coap_collector import Alarm
from coap_collector import AlarmSwitch


# Define Variables
MQTT_HOST = "localhost"
MQTT_PORT = 1883
MQTT_KEEPALIVE_INTERVAL = 45
MQTT_TOPIC = "info"
MQTT_MSG = "hello MQTT"

ip = "::"
port = 5683

'''
    Class CoAPServer. Simply instantiate a CoAPServer (extends base class CoAP) and add a resource 
    AdvancedResource, to manage registrations
'''
class CoAPServer(CoAP):
    def __init__(self, host, port):
        CoAP.__init__(self, (host, port), False)
        # Register resource: server behave as client in order to get the registration
        print("adding resource");
        self.add_resource("registration", AdvancedResource())
        # self.add_resource("registration", AdvancedResourceAlert())
        # self.add_resource("registrationAlert", AdvancedResourceAlertSwitch())

class MqttClient():
    # Define on connect event function
    # We shall subscribe to our Topic in this function
    def on_connect(self, client, mosq, obj, rc):
        # mqttc.subscribe(MQTT_TOPIC, 0)
        self.client.subscribe(MQTT_TOPIC, 0)

    # Define on_message event function.
    # This function will be invoked every time,
    # a new message arrives for the subscribed topic
    def on_message(self, client, userdata, msg):
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
        ts = time.time()
        timestamp = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
        # faccio la connessione a sql e metto i dati, nella nostra app posso anche evitare --> posso usare un df
        with self.connection.cursor() as cursor:
            # Create a new record
            sql = "INSERT INTO `mqttsensors` (`temperature`, `humidity`,`light` ,`gas`, `timestamp`) VALUES (%s, %s, %s , %s, %s)"
            cursor.execute(sql, (temp, humidity, light, gas, timestamp))
            print("temp : ")
            print(temp)
            print("humidity : ")
            print(humidity)


        # Commit changes
        self.connection.commit()

        with self.connection.cursor() as cursor2:
            sql = "SELECT * FROM `mqttsensors`"
            cursor2.execute(sql)
            results = cursor2.fetchall()
            header = results[0].keys()
            rows = [x.values() for x in results]
            print(tabulate.tabulate(rows,header,tablefmt='grid'))

    def mqtt_client(self):
        self.db = Database()
        self.connection = self.db.connect_dbs()
        print("Mqtt client starting")
        self.client = mqtt.Client()
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        try:
            self.client.connect(MQTT_HOST, MQTT_PORT, MQTT_KEEPALIVE_INTERVAL)
            print("Connected\n")
        except Exception as e:
            print(str(e))
        self.client.loop_forever()


logging.getLogger("coapthon.server.coap").setLevel(logging.WARNING)
logging.getLogger("coapthon.layers.messagelayer").setLevel(logging.WARNING)
logging.getLogger("coapthon.client.coap").setLevel(logging.WARNING)


# Initiate MQTT Client

#mqttc = mqtt.Client()
#mqttc.db = Database()
#mqttc.connection = mqttc.db.connect_dbs()

# Assign event callbacks
#mqttc.on_message = on_message
#mqttc.on_connect = on_connect
# Connect with MQTT Broker
mqttc = MqttClient();
# mqttc.connect(MQTT_HOST, MQTT_PORT, MQTT_KEEPALIVE_INTERVAL)
mqtt_thread = threading.Thread(target=mqttc.mqtt_client,args=(),kwargs={})
# mqtt_thread.daemon = True
mqtt_thread.start()
# Continue monitoring the incoming messages for subscribed topic
#mqttc.loop_forever()
server = CoAPServer(ip, port)
try:
    print("Listening to server")
    server.listen(100)
except KeyboardInterrupt:
    print("Server Shutdown")
    mqttc.kill()
    mqttc.join()
    server.close()
    print("Exiting...")
mqttc.loop_forever()