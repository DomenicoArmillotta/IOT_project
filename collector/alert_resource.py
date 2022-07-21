from coapthon.server.coap import CoAP
from coapthon.resources.resource import Resource
from coapthon.client.helperclient import HelperClient
from coapthon.messages.request import Request
from coapthon.messages.response import Response
from coapthon import defines
import json
import time
import server
import threading
from database import Database
import tabulate
import logging

class AlertResource :
    def __init__(self,source_address,resource):
        # Initialize mote resource fields
        self.db = Database()
        self.connection = self.db.connect_dbs()
        self.address = source_address
        self.resource = resource
        self.actuator_resource = "alert_actuator"
        self.intensity = 10;
        self.isActive = "F";
        # Start observing for updates
        self.start_observing()
        print("Alert resource initialized")

    # salvo l'intensita' dell'allarme
    def presence_callback_observer(self, response):
        print("Callback called, resource arrived")
        if response.payload is not None:
            print(response.payload)
            nodeData = json.loads(response.payload)
            # read from payload of client
            info = nodeData["info"].split(" ")
            intensity = nodeData["intensity"].split(" ")
            print("Detection value :")
            print(info)
            print(intensity)
            self.isDetected = info[0]
            self.intensity = intensity[0];
            # when occour an intrusion a query is executed
            if self.isActive == 'T':
                self.execute_query()


    def execute_query(self):
        print(self.connection)
        with self.connection.cursor() as cursor:
            # Create a new record
            # ts = time.time()
            # time = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
            intensity = str(self.intensity)
            # sql = "INSERT INTO `coapsensorsalarm` (`value`, `timestamp`, `intensity`) VALUES (%s, %s , %s)"
            # cursor.execute(sql, (self.isDetected, time, intensity))
            sql = "INSERT INTO `coapsensorsalarm` (`value`, `intensity`) VALUES (%s, %s)"
            cursor.execute(sql, (self.isDetected, intensity))
        # connection is not autocommit by default. So you must commit to save
        # your changes.
        self.connection.commit()




    #non e' efficente fare query al db per mettere in store quando e' acceso l'allarme
    def start_observing(self):
        logging.getLogger("coapthon.server.coap").setLevel(logging.WARNING)
        logging.getLogger("coapthon.layers.messagelayer").setLevel(logging.WARNING)
        logging.getLogger("coapthon.client.coap").setLevel(logging.WARNING)
        self.client = HelperClient(self.address)
        self.client.observe(self.resource,self.presence_callback_observer)
