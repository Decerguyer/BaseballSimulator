#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Jul 13 23:19:18 2020

@author: yonatanarieh
"""

#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Jul  1 16:45:36 2020

@author: yonatanarieh
"""
import math
import csv
import json
#Speed, Initial Position, Vertical plane angle, Horizontal plane angle, backspin, sidespin, gyroscopic spin
#Note that position is coordinates in feet with respect to the back corner of home plate which is (0,0,0)
#Angles are given in degrees, Spin is given in rpm
def baseballTrajectory(Xi,Yi,Zi,Vinx,Viny,Vinz,wb,ws,wg):
    print(Xi,Yi,Zi,Vinx,Viny,Vinz,wb,ws,wg)
    baseballMass = 5.125 #Oz
    baseballCircumference = 9.125 #inches
    rho = 0.0740 #Air density at (75 F) (50% Humidity) (760 mm Hg pressure) (0 elevation)
    const = 0.005316 #5.316E-03 = 0.07182*rho*(5.125/mass)*(circ/9.125)^2
    #Edit values for mass, circ, or rho to find new const if necessary
    Cd = 0.330 #Drag Coefficient
    #beta = 0.0001217 #Constant in calculating actual pressure not used in this script just for reference
    #SVP constant (Saturation Vapor Pressure)
    #theta = theta*(math.pi/180) #Convert to radians
    #phi = phi*(math.pi/180) #Convert to radians



    #Vin = mph*1.467
    #Vinz = Vin*math.sin(theta) #Takes vertical component of throw
    #Vinx = Vin*math.cos(theta)*math.sin(phi) #Takes horizontal component of throw then adjusts on horizontal plane. X axis = 1st base-third base line
    #Viny = Vin*math.cos(theta)*math.cos(phi)#Takes horizontal component of throw then adjusts on horizontal plane. Y axis = Home plate to pitching mound line
    Vin = math.sqrt(Vinx**2 + Viny**2 + Vinz**2)


    theta = math.asin(Vinz/Vin)
    phi = math.acos(Viny/(Vin*math.cos(theta)))
    print(math.degrees(theta),math.degrees(phi))
    '''
    theta = -2*math.pi/180
    phi= 179*math.pi/180
    print(math.degrees(theta),math.degrees(phi))
    '''

    wx = (wb*math.cos(phi)-ws*math.sin(theta)*math.sin(phi)+wg*Vinx/Vin)*math.pi/30
    wy = (-wb*math.sin(phi)-ws*math.sin(theta)*math.cos(phi)+wg*Viny/Vin)*math.pi/30
    wz = (ws*math.cos(theta)+wg*Vinz/Vin)*math.pi/30

    w = math.sqrt(wx**2 + wy**2 + wz**2) #rad per sec
    rw =(baseballCircumference/(2*math.pi))*(w/12) #ft per sec

    vxw = 0 #Wind speed in x direction ft/sec
    vyw = 0 #Wind speed in y direction ft/sec


    #Re_100 = 210000 #2.100E+05 Reynolds number for 100 mph throw

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
    #print(positionVector)
    with open('FullSystemSimulation.csv', 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerows(positionVector)
        #writer.writerows(velocityVector)
        #writer.writerows(accelerationVector)

xdata = []
ydata = []
zdata = []

vxdata = []
vydata = []
vzdata = []

with open('KalmanFilterPositionSmooth.csv', newline='') as f:
    reader = csv.reader(f)
    for row in reader:
        None
    xdata.append(float(row[0]))
    ydata.append(float(row[1]))
    zdata.append(float(row[2]))
    vxdata.append(float(row[3]))
    vydata.append(float(row[4]))
    vzdata.append(float(row[5]))

spin = []
with open('Spin.csv', newline='') as f:
    reader = csv.reader(f)
    for row in reader:
        spin.append(float(row[0]))

a = 0
netDistance = 10
netPosition = 55-netDistance
for i in range(0,len(ydata)):
    #print(ydata[i])
    if(ydata[i] < netPosition):
        a = i
        break

#actualInitial= [-0.792482447, 44.92264101,5.610230194,3.248701609,-136.8477863,-5.817849185,10.95954332,32.42372471,-13.19278932]
#Xi = actualInitial[0]
#Yi = actualInitial[1]
#Zi = actualInitial[2]
#Vinx = actualInitial[3]
#Viny = actualInitial[4]
#Vinz = actualInitial[5]

Xi = xdata[a]
Yi = ydata[a]
Zi = zdata[a]
Vinx = vxdata[a]
Viny = vydata[a]
Vinz = vzdata[a]

#print(spin)
baseballTrajectory(Xi,Yi,Zi,Vinx,Viny,Vinz,spin[0],spin[1],spin[2])




        
        
        
        
        
        
    
    
    