#!/usr/bin/env python
import getopt
import sys
import json
import threading
import time
from coapthon.server.coap import CoAP
from coapthon.resources.resource import Resource
from coapthon.client.helperclient import HelperClient
from coapthon.messages.request import Request
from coapthon.messages.response import Response
from coapthon import defines
from alert_resource import AlertResource
from motion_resource import MotionResource

# Contain all the registered users
registeredUsersDict = {}

'''
    Advanced resource: manage automatically response to registration request. Creates a MoteResource object 
    each time a lock sensor register through the CoAPServer
'''

class AdvancedResource(Resource):

    def __init__(self, name="Advanced"):
        super(AdvancedResource, self).__init__(name)
        self.payload = "Successful registration"
    def render_GET_advanced(self, request, response):
        print("GET server, received message:\n")
        print(request.payload)
        # Store the (id,port) request: print(request.source)
        # Now, we extract the information from the json payload
        moteInfo = json.loads(request.payload)
        # Send a response with successful outcome
        response.payload = self.payload
        print(response.payload)
        response.max_age = 20
        response.code = defines.Codes.CONTENT.number
        # Memorize source in dict to know destination address
        moteInfo["Source"] = request.source

        motionResource = MotionResource(moteInfo["Source"],moteInfo["Resource"])
        return self, response

class AdvancedResourceAlert(Resource):

    def __init__(self, name="AdvancedAlert"):
        super(AdvancedResourceAlert, self).__init__(name)
        self.payload = "Successful registration"

    def render_GET_advanced(self, request, response):
        print("GET server, received message:\n")
        print(request.payload)
        # Store the (id,port) request: print(request.source)
        # Now, we extract the information from the json payload
        moteInfo = json.loads(request.payload)
        # Send a response with successful outcome
        response.payload = self.payload
        print(response.payload)
        response.max_age = 20
        response.code = defines.Codes.CONTENT.number
        # Memorize source in dict to know destination address
        moteInfo["Source"] = request.source
        alertResource = AlertResource(moteInfo["Source"],moteInfo["Resource"])
        return self, response

