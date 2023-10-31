#!/usr/bin/env python3
import serial
import time
import numpy as np
from sendStringScript import sendString
isCross=int(0)
crossCount=int(0)
leftMotor=int(100)
rightMotor=int(100)


if __name__ == '__main__':
    ser=serial.Serial('/dev/ttyACM2',115200) #sometimes is /dev/ttyACM0' or ttyACM02 unsure what deliniates
    ser.reset_input_buffer() #clears anything the arduino has been sending while the Rpi isnt prepared to recieve.

    while True:
        sendString('/dev/ttyACM2',115200,'<'+str(leftMotor)+','+str(rightMotor)+'>',0.0001)
        #ser.write(('<'+str(leftMotor)+str(rightMotor)+'>').encode())
        if ser.in_waiting > 0:  #we wait until the arduino has sent something to us before we try to read anything from the serial port.
                 
            #first we read the data from the arduino using the pySerial library
            recievedData= ser.readline().decode('utf-8')
            ser.reset_input_buffer() #clears anything the arduino has been sending while the Rpi isnt prepared to recieve.

            #this below variable keeps track of what isCross was in the previos iteration of the whileloop, will be used later
            isCrossLastTimestep=isCross

            #now we split that data up into a list, using commas to deliniate between elements
            try:
                recievedData=recievedData.split(',')
                linePose=int(recievedData[0])
                isCross=int(recievedData[1])
                leftMotorrcvd=recievedData[2]
                rightMotorrcvd=recievedData[3]
            except:
                print("packet lost")

            #incrimenting seeing a cross: we only want to count each cross once, so we will use crossCount changing from 0->1 as our inclination that we have hit a new cross
            #this will prevent us from counting the same cross twice
            if isCrossLastTimestep == 0 and isCross ==1:
                 #meaning we were NOT on a cross last timestep but ARE on a cross now. this makes sure we dont count the same cross more than once
                crossCount=crossCount+1
            
            
            #now that we store our position on the line as well as how many crosses we have hit, we can write a state machine, which can incriment based on crossCount.
            #STATE 1: WE HAVENT HIT THE SECOND CROSS YET, we translate forward using proportonal control
            if crossCount < 2:
                #rightMotor=int(((linePose-1000)/4000)*300 + 100) #maps leftmotor to be between 100 and 400, will be LOW if robot is to thr right of the line. keep these as ints, less bits being sent the better
                #leftMotor=int(400-(((linePose-1000)/4000)*300))
                rightMotor=int(np.interp(linePose,[1000,5000],[100,400])-40) #interpelation functions that map our linesenor values to motorcommand values, made convient by the numpy library
                leftMotor=int(np.interp(linePose,[1000,5000],[400,100])+40)

            if crossCount==2: #rotates the robot to the right until the line sensor lines up with the new line, which incriments crossCounter and moves me into my next state
                rightMotor=0
                leftMotor=0
                print("reached second cross")
                #time.sleep(500) #these pauses are for me to be able to clearly see when I change states
                rightMotor=-150
                leftMotor=150
            
            if crossCount ==3: #we should be facing the right side of the board now, and we want to translate forward until we hit the next cross
                #time.sleep(1)
                rightMotor=int(np.interp(linePose,[1000,5000],[100,400])-40)
                leftMotor=int(np.interp(linePose,[1000,5000],[400,100])+40)
                
            
            if crossCount ==4: #now we've hit the right front cross, lets rotate to the left until the line sensor alignes with the line paralell to the goal board and causing the crossCount to incriment, which will exit us out of this state and into the next one
                #time.sleep(1)
                rightMotor=150
                leftMotor=-150
            
            if crossCount > 4: #once we have alighned with the line paralell to the goal board, we stop.
                rightMotor=0
                leftMotor=0
            
            print('left:'+str(leftMotorrcvd)+'right:'+str(rightMotorrcvd))
            print(linePose)
            
            #     #Following is my control law, we're keeping it basic for now, writing good control law is your job
            #     #ok so high numbers(highest 7000) on the line follwing mean I am too far to the RIGHT,
            #     #low numbers mean I am too far on the RIGHT, 3500 means I am at the middle
            #     #below is a basic control law you can send to your motors, with an exeption if z is a value greater than 7000, meaning the arduino code sees that the line sensor is on a cross. Feel free to take insperation from this,
            # #but you will need to impliment a state machine similar to what you made in lab 2 (including a way of counting time without blocking)
            
            #     if x < 70001: #im assuming that in your arduino code you will be setting z to the int 8000 if you sense a cross, dont feel obligated to do it this way.  
            #         leftMotor=400-((x/7000)*300) #will be between 100 and 400, HIGh if z is low
            #         rightMotor=((x/7000)*300)+100 #will be between 100 and 400, LOW if z is low
            #     else:
            #         print('at intersetion')
            #         #do something here like incrimenting a value you call 'lines_hit' to one higher, and writing code to make sure that some time (1 second should do it) 
            #         # passes between being able to incriment lines_hit so that it wont be incrimented a bunch of times when you hit your first cross. IE give your robot time to leave a cross
            #         #before allowing lines_hit to be incrimented again.
