#!/usr/bin/env python
import getopt
import sys
from coapthon.server.coap import CoAP
from coapthon.resources.resource import Resource
import socket
from coapthon.client.helperclient import HelperClient
from coapthon.utils import parse_uri

class Motion(Resource):
    def __init__(self, name="Motion", coap_server=None):
        super(Motion, self).__init__(name, coap_server, visible=True, observable=True, allow_children=True)
        self.payload = "Motion Resource"
        self.resource_type = "motion_sensor"
        self.content_type = "text/plain"
        self.interface_type = "if1"

class Alarm(Resource):
    def __init__(self, name="Alarm", coap_server=None):
        super(Alarm, self).__init__(name, coap_server, visible=True, observable=True, allow_children=True)
        self.payload = "Alarm Resource"
        self.resource_type = "alert_actuator"
        self.content_type = "text/plain"
        self.interface_type = "if1"


class CoAPServer(CoAP):
    def __init__(self, host, port):
        CoAP.__init__(self, (host, port), False)
        self.add_resource("Motion/", Motion())
        self.add_resource("Alarm/", Alarm())



ip = "0.0.0.0"
port = 5683


server = CoAPServer(ip, port)

try:
    server.listen(10)
except KeyboardInterrupt:
    print("Server Shutdown")
    server.close()
    print("Exiting...")