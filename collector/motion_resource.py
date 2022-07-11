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
from datetime import datetime
from alert_resource import AlertResource

class MotionResource :
    def __init__(self,source_address,resource):
        # Initialize mote resource fields
        self.db = Database()
        self.connection = self.db.connect_dbs()
        self.address = source_address
        self.resource = resource
        self.actuator_resource = "alert_actuator"
        self.isDetected = "F";
        # Start observing for updates
        self.start_observing()

    def detection_callback_observer(self, response):
        print("Callback called, resource arrived")
        if response.payload is not None:
            print(response.payload)
            nodeData = json.loads(response.payload)
            #read from payload of client
            isDetected = nodeData["isDetected"].split(" ")
            print("Detection value :")
            print(isDetected)
            self.isDetected = isDetected[0]
            #when occour an intrusion a query is executed
            if self.isDetected == 'T':
                #response per far cambiare stato all'alert
                response = self.client.post(self.actuator_resource,"ON")
                #faccio la query quando trovo un intruso
                self.execute_query(1)
            else:
                #quando non c'è un intruso cambio solo lo stato , ma senza query
                response = self.client.post(self.actuator_resource,"OFF")



    def execute_query(self,value):

        print(self.connection)

        if value == 1:

            with self.connection.cursor() as cursor:
                # Create a new record
                time = datetime.now().strftime("%m/%d/%Y, %H:%M:%S")
                sql = "INSERT INTO `coapsensorsMotion` (`value`, `timestamp`) VALUES (%s, %s)"
                cursor.execute(sql, (self.isDetected, time))

            # connection is not autocommit by default. So you must commit to save
            # your changes.
            self.connection.commit()



    def start_observing(self):
        logging.getLogger("coapthon.server.coap").setLevel(logging.WARNING)
        logging.getLogger("coapthon.layers.messagelayer").setLevel(logging.WARNING)
        logging.getLogger("coapthon.client.coap").setLevel(logging.WARNING)
        self.client = HelperClient(self.address)
        self.client.observe(self.resource,self.presence_callback_observer)