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
        self.intensity = 10;
        self.isActive = "F";
        # Start observing for updates
        self.start_observing()
        print("Motion resource initialized")

    def presence_callback_observer(self, response):
        print("Callback called, resource arrived")
        print(response.payload)
        if response.payload is not None:
            print(response.payload)
            nodeData = json.loads(response.payload)
            # read from payload of client
            isDetected = nodeData["isDetected"].split(" ")
            info = nodeData["info"].split(" ")
            intensity = nodeData["intensity"].split(" ")
            print("Detection value motion node :")
            print(isDetected)
            print(info)
            print(intensity)
            self.isDetected = isDetected[0]
            self.isActive = info[0]
            self.intensity = intensity[0];
            # when occour an intrusion a query is executed
            if self.isDetected == 'T':
                response = self.client.post(self.actuator_resource,"state=1")
                print("Funziona 1a")
                # faccio la query quando trovo un intruso
                self.execute_query_motion(1)
            else:
                response = self.client.post(self.actuator_resource,"state=0")
                print("Funziona 1b")
                self.execute_query_motion(0)
        else:
            return;



    def execute_query_motion(self,value):

        print(self.connection)
        with self.connection.cursor() as cursor:
            # Create a new record
            # ts = time.time()
            # time = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
            intensity = str(self.intensity)
            alarm = str(self.isActive)
            sql = "INSERT INTO `coapsensorsmotion` (`value`,`alarm`,`intensity`) VALUES (%s,%s,%s)"
            cursor.execute(sql, (value,alarm,intensity))

        # connection is not autocommit by default. So you must commit to save
        # your changes.
        self.connection.commit()
        # Show data log
        with self.connection.cursor() as cursor2:
            sql = "SELECT * FROM `coapsensorsmotion`"
            cursor2.execute(sql)
            results = cursor2.fetchall()
            header = results[0].keys()
            rows = [x.values() for x in results]
            print(tabulate.tabulate(rows,header,tablefmt='grid'))



    def start_observing(self):
        logging.getLogger("coapthon.server.coap").setLevel(logging.WARNING)
        logging.getLogger("coapthon.layers.messagelayer").setLevel(logging.WARNING)
        logging.getLogger("coapthon.client.coap").setLevel(logging.WARNING)
        self.client = HelperClient(self.address)
        self.client.observe(self.resource,self.presence_callback_observer)
