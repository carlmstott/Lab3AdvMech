#!/usr/bin/env python3
import serial
import time
import numpy as np
import matplotlib as plt
from sendStringScript import sendString
leftMotor=int(100)
rightMotor=int(100)


if __name__ == '__main__':
    ser=serial.Serial('/dev/ttyACM0',115200)
    #every time the serial port is opened, the arduino program will restart
    ser.reset_input_buffer()
    ready = 0

    while True:
        if ready == 0:
            line = ser.readline().decode('utf-8')
            print(line)
            ready = 1 #we wait until arduino has sent its first full line (<arduino is ready>) line before sending anything to arduino, this gives arduino time to set up, np matter how long that time is
                      


        if ready ==1:
            sendString('/dev/ttyACM0',115200,'<'+str(leftMotor)+','+str(rightMotor)+'>',0.0001)
            # print('<'+str(leftMotor)+','+str(rightMotor)+'>')
            if ser.in_waiting > 0:  
                 
                line = ser.readline().decode('utf-8')
                # print(line)
                line=line.split(',')
                # print(line)
                # x=line[0]
                # print(x)
                #readline reads one line including the /n, technically this is blocking code but it matters much less to have blocking code on the python side
                try:
                    
                    x=int(line[0])
                    y=int(line[1])
                    z=float(line[2])
                    print([x,y,z])
                    # linePos=float(line)
                except:
                    print("whoopsie") #this is designed to catch when python shoves bits on 
                                      #top of each other. 
                #Following is my control law, we're keeping it basic for now, writing good control law 
                #the students job
                #ok so high numbers(highest 7000) on the line follwing mean I am too far to the LEFT,
                #low numbers mean I am too far on the RIGHT, 3500 means I am at the middle
                leftMotor=100+.02*z
                rightMotor=250-.02*z