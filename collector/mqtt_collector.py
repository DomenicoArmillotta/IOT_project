import paho.mqtt.client as mqtt
import json
from database import Database
import tabulate
#USIAMO Mosquitto COME BROKER SULLA VM




'''
    Class MqttClient: opens a connection through the mqtt Broker and register interest for the topic weatherinfo. 
    Each time a message is published, the on_message is called and the database is updated. 
    Finally data log is showed
'''
class MqttClient:

    ''' 
        The callback for when the client receives a CONNACK response from the server. Called as soon as 
        the connection with the MQTT broker is established. The client subscribe in this moment to the topic 
        weatherinfo to receive environmental updates
    '''
    def on_connect(self, client, userdata, flags, rc):
        print("Connected with result code "+str(rc))
        #topic a cui mi voglio iscrivere
        self.client.subscribe("info")

    ''' 
        The callback for when a PUBLISH message is received from the server. The received payload is inserted into the database
        and finally the datalog is showed in tabular data
    '''
    def on_message(self, client, userdata, msg):
        print(msg.topic+"  "+str(msg.payload))
        receivedData = json.loads(msg.payload)
        temp = receivedData["temp"]
        umidity = receivedData["umidity"]
        light = receivedData["light"]
        gas = receivedData["gas"]
        if receivedData["light"] == 0:
            light = "LOUMINOUS"
        elif receivedData["light"] == 1:
            light = "RELAX"
        else:
            light = "DARK"
        #faccio la connessione a sql e metto i dati, nella nostra app posso anche evitare --> posso usare un df
        with self.connection.cursor() as cursor:
            # Create a new record
            sql = "INSERT INTO `mqttsensors` (`temperature`, `umidity`,`light` ,`gas`) VALUES (%s, %s, %s , %s)"
            cursor.execute(sql, (temp,umidity,light , gas))

        # Commit changes
        self.connection.commit()

        # Show data log
        with self.connection.cursor() as cursor2:
            sql = "SELECT * FROM `mqttsensors`"
            cursor2.execute(sql)
            results = cursor2.fetchall()
            header = results[0].keys()
            rows = [x.values() for x in results]
            print(tabulate.tabulate(rows,header,tablefmt='grid'))


    '''
        Method started by the thread. Database connection is opened, and connection to the remote MQTT broker 
        (through local port forwarding) is established
    '''
    def mqtt_client(self):
        self.db = Database()
        self.connection = self.db.connect_dbs()
        print("Mqtt client starting")
        self.client = mqtt.Client()
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        try:
            self.client.connect("127.0.0.1", 1883, 60)
        except Exception as e:
            print(str(e))
        self.client.loop_forever()
