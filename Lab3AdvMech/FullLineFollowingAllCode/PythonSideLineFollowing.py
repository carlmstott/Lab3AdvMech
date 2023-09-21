#!/usr/bin/env python3
import serial
import time
import numpy as np
from sendStringScript import sendString
leftMotor=int(100)
rightMotor=int(100)


if __name__ == '__main__':
    ser=serial.Serial('/dev/ttyACM0',115200)
    ser.reset_input_buffer() #clears anything the arduino has been sending while the Rpi isnt prepared to recieve.

    while True:
        sendString('/dev/ttyACM0',115200,'<'+str(leftMotor)+','+str(rightMotor)+'>',0.0001)
        
        if ser.in_waiting > 0:  #we wait until the arduino has sent something to us before we try to read anything from the serial port.
                 
                line = ser.readline().decode('utf-8')
                line=line.split(',')
                #this splits the incoming string up by commas
                try:
                    
                    x=int(line[0])
                    y=int(line[1])
                    z=line[2] #we dont convert this to a float becasue we went to be able to recieve the message that we are at a cross, which wont be an int. 
                    print([x,y,z])
                except:
                    print("packet dropped") #this is designed to catch when python shoves bits on 
                                          #top of each other. 
            
                #Following is my control law, we're keeping it basic for now, writing good control law is your job
                #ok so high numbers(highest 7000) on the line follwing mean I am too far to the LEFT,
                #low numbers mean I am too far on the RIGHT, 3500 means I am at the middle
                #below is a basic control law you can send to your motors, with an exeption if z is the string 'cross' and not a lineposition
            
                if not z == 'cross': #im assuming that in your arduino code you will be setting z to the string 'cross' if you sense a cross, dont feel obligated to do it this way.  
                    leftMotor=100+.02*int(z) #now that we are SURE that z isbt the string cross, we cast z to an int and recalculate leftMotor and rightMotor, 
                    rightMotor=250-.02*int(z)
                #else:
                    #do something here like incrimenting a value you call 'lines_hit' to one higher, and writing code to make sure that some time (1 second should do it) 
                    # passes between being able to incriment lines_hit so that it wont be incrimented a bunch of times when you hit your first cross. IE give your robot time to leave a cross
                    #before allowing lines_hit to be incrimented again.
