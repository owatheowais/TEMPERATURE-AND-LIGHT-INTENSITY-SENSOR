#!/usr/bin/python

# this source is part of my Hackster.io project:  https://www.hackster.io/mariocannistra/radio-astronomy-with-rtl-sdr-raspberrypi-and-amazon-aws-iot-45b617

# use this program to test the AWS IoT certificates received by the author
# to participate to the spectrogram sharing initiative on AWS cloud

# this program will publish test mqtt messages using the AWS IoT hub
# to test this program you have to run first its companion awsiotsub.py
# that will subscribe and show all the messages sent by this program

import paho.mqtt.client as paho
import os
import socket
import ssl
from time import sleep
from random import uniform

connflag = False

def on_connect(client, userdata, flags, rc):
    global connflag
    connflag = True
    print("Connection returned result: " + str(rc) )

def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

#def on_log(client, userdata, level, buf):
#    print(msg.topic+" "+str(msg.payload))

mqttc = paho.Client()
mqttc.on_connect = on_connect
mqttc.on_message = on_message
#mqttc.on_log = on_log

awshost = "a1t7wpltysp7gc.iot.us-east-2.amazonaws.com"
awsport = 8883
clientId = "Iot1"
thingName = "Iot1"
caPath = 'VeriSign-Class 3-Public-Primary-Certification-Authority-G5.pem.txt'
certPath = 'abeeb26700-certificate.pem.crt'
keyPath = 'abeeb26700-private.pem.key'

mqttc.tls_set(caPath, certfile=certPath, keyfile=keyPath, cert_reqs=ssl.CERT_REQUIRED, tls_version=ssl.PROTOCOL_TLSv1_2, ciphers=None)

mqttc.connect(awshost, awsport, keepalive=60)

mqttc.loop_start()

for i in range(50):
    sleep(0.5)
    if connflag == True:
        topic = input("Please enter iot code to publish:\n")
        message = input("Please write message to publish:\n")
        mqttc.publish(topic, message, qos=1)
        print("\nmsg sent")
    else:
        print("waiting for connection...")
