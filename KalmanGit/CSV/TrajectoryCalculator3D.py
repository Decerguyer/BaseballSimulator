#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Jul  1 16:45:36 2020

@author: yonatanarieh
"""
import math
import csv
from numpy.random import randn
#Speed, Initial Position, Vertical plane angle, Horizontal plane angle, backspin, sidespin, gyroscopic spin
#Note that position is coordinates in feet with respect to the back corner of home plate which is (0,0,0)
#Angles are given in degrees, Spin is given in rpm
def baseballTrajectory(mph,Xi,Yi,Zi,theta,phi,wb,ws,wg):
    
    baseballMass = 5.125 #Oz
    baseballCircumference = 9.125 #inches
    rho = 0.0740 #Air density at (75 F) (50% Humidity) (760 mm Hg pressure) (0 elevation)
    const = 0.005316 #5.316E-03 = 0.07182*rho*(5.125/mass)*(circ/9.125)^2
                     #Edit values for mass, circ, or rho to find new const if necessary
    Cd = 0.330 #Drag Coefficient
    #beta = 0.0001217 #Constant in calculating actual pressure not used in this script just for reference
    #SVP constant (Saturation Vapor Pressure)
    theta = theta*(math.pi/180) #Convert to radians
    phi = phi*(math.pi/180) #Convert to radians
    
    if(spin == []):
        spin.append(wb+randn()*50)
        spin.append(ws+randn()*50)
        spin.append(wg+randn()*50)
    
    Vin = mph*1.467
    Vinz = Vin*math.sin(theta) #Takes vertical component of throw
    Vinx = Vin*math.cos(theta)*math.sin(phi) #Takes horizontal component of throw then adjusts on horizontal plane. X axis = 1st base-third base line
    Viny = Vin*math.cos(theta)*math.cos(phi)#Takes horizontal component of throw then adjusts on horizontal plane. Y axis = Home plate to pitching mound line
    
    wx = (wb*math.cos(phi)-ws*math.sin(theta)*math.sin(phi)+wg*Vinx/Vin)*math.pi/30
    wy = (-wb*math.sin(phi)-ws*math.sin(theta)*math.cos(phi)+wg*Viny/Vin)*math.pi/30
    wz = (ws*math.cos(theta)+wg*Vinz/Vin)*math.pi/30
   
    w = math.sqrt(wx**2 + wy**2 + wz**2) #rad per sec
    rw =(baseballCircumference/(2*math.pi))*(w/12) #ft per sec
    
    vxw = 0 #Wind speed in x direction ft/sec
    vyw = 0 #Wind speed in y direction ft/sec
    
    
    Re_100 = 210000 #2.100E+05 Reynolds number for 100 mph throw
    
    positionVector = []
    velocityVector = []
    accelerationVector = []
    
    initialPos = [Xi,Yi,Zi]
    positionVector.append(initialPos)
    
    initialVel = [Vinx,Viny,Vinz]
    velocityVector.append(initialVel)
    t = 0.
    dt = 0.0001
    tau = 10000 #Spin decay constant. Set large so spin doesn't decay much
    while(positionVector[-1][1] > 1.4 and positionVector[-1][2] > 0.4): #Run trajectory while the ball is front of home plate and the ball is 4.8 inches above the ground
        
        vw = math.sqrt((velocityVector[-1][0]-vxw)**2 + (velocityVector[-1][1]-vyw)**2 + (velocityVector[-1][2])**2) #Wind adjusted Velocity
        
        aDragx = const*(velocityVector[-1][0]-vxw)*Cd*vw*-1
        aDragy = const*(velocityVector[-1][1]-vyw)*Cd*vw*-1
        aDragz = const*(velocityVector[-1][2])*Cd*vw*-1
        
        S = (rw/vw)*(math.e**((t*-1)/tau))
        Cl = 1.0/(2.32+(0.4/S))
        aMagx = const*vw*(wy*velocityVector[-1][2]-wz*(velocityVector[-1][1]-vyw))*(Cl/w)
        aMagy = const*vw*(wz*(velocityVector[-1][0]-vxw)-wx*(velocityVector[-1][2]))*(Cl/w)
        aMagz = const*vw*(wx*(velocityVector[-1][1]-vyw)-wy*(velocityVector[-1][0]))*(Cl/w)
        
        aX = aDragx+aMagx
        aY = aDragy+aMagy
        aZ = aDragz+aMagz - 32.174 #Gravity
        acceleration = [aX,aY,aZ]
        accelerationVector.append(acceleration)
        
        vX = velocityVector[-1][0] + aX*dt
        vY = velocityVector[-1][1] + aY*dt
        vZ = velocityVector[-1][2] + aZ*dt
        velocity = [vX,vY,vZ]
        velocityVector.append(velocity)
        
        pX = positionVector[-1][0] + vX*dt + 0.5*aX*dt*dt
        pY = positionVector[-1][1] + vY*dt + 0.5*aY*dt*dt
        pZ = positionVector[-1][2] + vZ*dt + 0.5*aZ*dt*dt
        position = [pX,pY,pZ]
        positionVector.append(position)
        
        t += dt #Increment time 

    #'''
    with open('TrajectoryDataPosition.csv', 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerows(positionVector)
    
    spinString = ([str(spin[0])],[str(spin[1])],[str(spin[2])])
    with open('Spin.csv', 'w', newline='') as f:
        writer = csv.writer(f,delimiter = ',')
        writer.writerows(spinString)
    
    
    
    noisySensorInput = []
    tempState = []
    netDistance = 10 #This is variable to change
    netPosition = Yi-netDistance
    timeStamp = 0
    for i in range(0,len(positionVector),167):
        #print(ydata[i])
        if(positionVector[i][1] < netPosition):
            break
        tempState.append(positionVector[i][0]+randn()*(0.1/3))
        tempState.append(positionVector[i][1]+randn()*(((10-positionVector[i][2])*0.01)/3))
        tempState.append(positionVector[i][2]+randn()*(0.1/3))
        #tempState.append(positionVector[i][0]+randn()*(0.1/3))
        #tempState.append(positionVector[i][1]+randn()*(((10-positionVector[i][2])*0.01)/3))
        #tempState.append(positionVector[i][2]+randn()*(0.1/3))
        tempState.append(timeStamp)
        noisySensorInput.append(tempState)
        tempState = []
        timeStamp += 0.0166
        
    with open('SimNoisySensorInput.csv', 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerows(noisySensorInput)
    
    with open('KalmanTestTrajectoryDataPosition.csv', 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerows(positionVector)
    
spin = []
#baseballTrajectory(95,-1,55,6,-2,179,1500,1000,0) #Pitch 1
#baseballTrajectory(82,-1,55,6,-2,179,1200,-800,0) #Pitch 2
baseballTrajectory(55,-1,55,6,6,179,1200,-800,0) #Pitch 1
