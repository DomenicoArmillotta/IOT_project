# IOT_project
# Smart House
## Library to install to run project:
`$ pip3 install tabulate` 

`$ pip3 install server`

`$ pip3 install databases`

`$ pip3 install logging`

`$ pip3 install nrfutil`

link to fix bug on nrfutil = 
[links](https://appuals.com/command-python-setup-py-egg_info/#:~:text=Fix%3A%20'Command%20%E2%80%9Cpython%20setup,code%201'%20When%20Installing%20Python&text=The%20error%20code%201%20is,to%20be%20installed%20or%20updated)


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
cd contiki-ng/IoTProject/sensors/coap-sensor
make TARGET=nrf52840 BOARD=dongle coap_sensor.dfu-upload PORT=/dev/ttyACM0
```

## Project Overview:
Our project aims to create a home monitoring system , environmental factors and an intrusion detection system. All the data are then saved in a database.
"mqtt" sensors take care of the monitoring part of gas , lighting , temperature and humidity.
"Coap" sensors deal with the intrusion system , as soon as the motion sensors sense the intruder , the alarm is triggered and gradually increases in intensity. To manually stop the alarm , just press the button of one of the alarms


![](/Documentation/image.png)
