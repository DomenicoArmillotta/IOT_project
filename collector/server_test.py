from server import *
from coapthon.server.coap import CoAP
import threading
from mqtt_collector import MqttClient
from coap_collector import Motion
from coap_collector import Alarm
from coap_collector import AlarmSwitch
import database
import logging

# Unspecified IPv6 address
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
        self.add_resource("Motion/", Motion())
        self.add_resource("Alarm/", Alarm())
        self.add_resource("AlarmSwitch/", AlarmSwitch())

'''
    Main function. Logging is suppressed, some users are generated with respective passwords 
    and finally CoAPServer and MQTT client thread are intialized
'''
def main():
    logging.getLogger("coapthon.server.coap").setLevel(logging.WARNING)
    logging.getLogger("coapthon.layers.messagelayer").setLevel(logging.WARNING)
    logging.getLogger("coapthon.client.coap").setLevel(logging.WARNING)

    print("Initializing server and MQTT client thread")
    mqttcl = MqttClient()
    # Before Initializing server, start a thread dedicated for the MQTT clients
    mqtt_thread = threading.Thread(target=mqttcl.mqtt_client,args=(),kwargs={})
    mqtt_thread.daemon = True
    mqtt_thread.start()
    # Start server on the main thread
    server = CoAPServer(ip, port)
    try:
        server.listen(100)
    except KeyboardInterrupt:
        print("Server Shutdown")
        mqtt_thread.kill()
        mqtt_thread.join()
        server.close()
        print("Exiting...")

if __name__ == '__main__':
    main()
