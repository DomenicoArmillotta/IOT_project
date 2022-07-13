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

    # salvo l'intensita' dell'allarme
    def detection_callback_observer(self, response):
        print("Callback called, resource arrived")
        if response.payload is not None:
            print(response.payload)
            nodeData = json.loads(response.payload)
            # read from payload of client
            info = nodeData["info"].split(" ")
            print("Detection value :")
            print(info)
            self.isDetected = info[0]
            self.intensity = info[1];
            # when occour an intrusion a query is executed
            if self.isActive == 'T':
                self.execute_query()


    def execute_query(self):
        print(self.connection)
        with self.connection.cursor() as cursor:
            # Create a new record
            time = datetime.now().strftime("%m/%d/%Y, %H:%M:%S")
            intensity = str(self.intensity)
            sql = "INSERT INTO `coapsensorsAlarm` (`value`, `timestamp` , `intensity`) VALUES (%s, %s , %s)"
            cursor.execute(sql, (self.isDetected, time , intensity))
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
