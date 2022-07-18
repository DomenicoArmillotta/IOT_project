#!/usr/bin/env python
import getopt
import sys
from coapthon.server.coap import CoAP
from coapthon.resources.resource import Resource
import threading
from mqtt_collector import MqttClient
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

class AlarmSwitch(Resource):
    def __init__(self, name="AlarmSwitch", coap_server=None):
        super(AlarmSwitch, self).__init__(name, coap_server, visible=True, observable=True, allow_children=True)
        self.payload = "Alarm Switch Resource"
        self.resource_type = "alert_switch_actuator"
        self.content_type = "text/plain"
        self.interface_type = "if1"

class CoAPServer(CoAP):
    def __init__(self, host, port):
        CoAP.__init__(self, (host, port), False)
        self.add_resource("Motion/", Motion())
        self.add_resource("Alarm/", Alarm())
        self.add_resource("AlarmSwitch/", AlarmSwitch())



#ip = "0.0.0.0"
#port = 5683

#print("Initializing server and MQTT client thread")
#mqttcl = MqttClient()
# Before Initializing server, start a thread dedicated for the MQTT clients
#mqtt_thread = threading.Thread(target=mqttcl.mqtt_client,args=(),kwargs={})
# mqtt_thread.daemon = True
#mqtt_thread.start()

#server = CoAPServer(ip, port)

#try:
#    server.listen(10)
#except KeyboardInterrupt:
#    print("Server Shutdown")
#    server.close()
#    print("Exiting...")