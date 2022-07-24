# IOT_project
# Smart House
## Terminal to run


Terminal to run Cooja 
```
contikier
cd tools/cooja
ant run
```
Terminal for Mosquitto:
```
sudo mosquitto -c /etc/mosquitto/mosquitto.conf
```
Terminal for Border router:
```
cd contiki-ng/IoTProject/sensors/rpl-border-router
make TARGET=cooja connect-router-cooja
```

Terminal for Python Server:
```
cd contiki-ng/IoTProject/collector/
python3 provaServer.py
```
