# IOT_project
# Smart House
## Library to install to run project:
`$ pip3 install tabulate`
`$ pip3 install server`
`$ pip3 install databases`
`$ pip3 install logging`
`$ pip3 install nrfutil`



## Terminal to run for Cooja emulation:


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
## How to flash Dongle card:
Flash Mqtt:
```
cd contiki-ng/IoTProject/sensors/mqtt-sensor
make TARGET=nrf52840 BOARD=dongle mqtt-client.dfu-upload PORT=/dev/ttyACM0
make TARGET=nrf52840 BOARD=dongle login PORT=/dev/ttyACM0
```
Flash Border router & run:
```
cd contiki-ng/IoTProject/sensors/rpl-border-router
make TARGET=nrf52840 BOARD=dongle border-router.dfu-upload PORT=/dev/ttyACM0
make TARGET=nrf52840 BOARD=dongle connect-router PORT=/dev/ttyACM0
```
Flash Coap:
```
```
