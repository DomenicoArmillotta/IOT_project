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
#class AdvancedResourceMotion(Resource):

  #  def __init__(self, name="Advanced"):
  #      super(AdvancedResourceMotion, self).__init__(name)
  #      self.payload = "Successful registration"

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
        response.max_age = 20
        response.code = defines.Codes.CONTENT.number
        # Memorize source in dict to know destination address
        moteInfo["Source"] = request.source

        motionResource = MotionResource(moteInfo["Source"],moteInfo["Resource"])
        return self, response

class AdvancedResourceAlert(Resource):

    def __init__(self, name="Advanced"):
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
        response.max_age = 20
        response.code = defines.Codes.CONTENT.number
        # Memorize source in dict to know destination address
        moteInfo["Source"] = request.source
        alertResource = AlertResource(moteInfo["Source"],moteInfo["Resource"])
        return self, response
'''
    def render_PUT_advanced(self, request, response):
        print("GET server, received message:\n")
        print(request.payload)
        # Store the (id,port) request: print(request.source)
        # Now, we extract the information from the json payload
        moteInfo = json.loads(request.payload)
        # Send a response with successful outcome
        response.payload = self.payload
        response.max_age = 20
        response.code = defines.Codes.CONTENT.number
        # Memorize source in dict to know destination address
        moteInfo["Source"] = request.source
        alertResource = AlertResource(moteInfo["Source"],moteInfo["MoteResource"],moteInfo["NodeID"],moteInfo["NodeType"])
        return self, response

    def render_POST_advanced(self, request, response):
        print("GET server, received message:\n")
        print(request.payload)
        # Store the (id,port) request: print(request.source)
        # Now, we extract the information from the json payload
        moteInfo = json.loads(request.payload)
        # Send a response with successful outcome
        response.payload = self.payload
        response.max_age = 20
        response.code = defines.Codes.CONTENT.number
        # Memorize source in dict to know destination address
        moteInfo["Source"] = request.source
        alertResource = AlertResource(moteInfo["Source"],moteInfo["MoteResource"],moteInfo["NodeID"],moteInfo["NodeType"])
        return self, response
'''

class AdvancedResourceAlertSwitch(Resource):

    def __init__(self, name="Advanced"):
        super(AdvancedResourceAlertSwitch, self).__init__(name)
        self.payload = "Successful registration"

    def render_GET_advanced(self, request, response):
        print("GET server, received message:\n")
        print(request.payload)
        # Store the (id,port) request: print(request.source)
        # Now, we extract the information from the json payload
        #TODO: json.loads raises an exception
        moteInfo = json.loads(request.payload)
        # Send a response with successful outcome
        print("Sending a response\n")
        response.payload = self.payload
        response.max_age = 20
        response.code = defines.Codes.CONTENT.number
        # Memorize source in dict to know destination address
        moteInfo["Source"] = request.source
        alertResource = AlertResource(moteInfo["Source"],moteInfo["Resource"])
        return self, response
