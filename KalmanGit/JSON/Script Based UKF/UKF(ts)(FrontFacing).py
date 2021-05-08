#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Jul 24 13:49:12 2020

@author: Yonatan Arieh
"""

import math
import csv
from filterpy.kalman import UnscentedKalmanFilter
from filterpy.kalman import unscented_transform, MerweScaledSigmaPoints
from filterpy.common import Q_discrete_white_noise
import numpy as np
from numpy.random import randn
import time as Time
import json

'''Begin Timer'''
start = Time.time()

def get_json(filepath):
    with open(filepath) as json_file:
        json_data = json.load(json_file)
    spin_dictionary = json_data["spin"]
    pitcher = json_data["pitcherId"]
    data_dictionary = json_data["positions"]
    data_array = []
    for position in data_dictionary:
        sub_arr = []
        sub_arr.append(position["x"])
        sub_arr.append(position["y"])
        sub_arr.append(position["z"])
        sub_arr.append(position["timestamp"])
        data_array.append(sub_arr)

    spin = [spin_dictionary["x"], spin_dictionary["y"], spin_dictionary["z"]]
    return data_array, data_dictionary, spin, spin_dictionary, pitcher

'''State Transition function for UKF (Determines the physical model)'''
#x = state, dt = dt, time = array with time value (1 element array only needs current value)
#Assumes acc, vel and pos in state are in ft/s**2, ft/s and ft
#Assumes spin components are in RPM and remain constant throughout
def fx(x,dt): #Generalized State transition function
#def fx(x,dt,wb = 1500+randn()*200,ws = 1000+randn()*200,wg = 0): #State transition function Pitch 1
#def fx(x,dt,wb = 1200+randn()*200,ws = -800+randn()*200,wg = 0): #State transition function Pitch 2
    wb = spin[0]
    ws = spin[1]
    wg = spin[2]
    
    #baseballMass = 5.125 #Oz
    baseballCircumference = 9.125 #inches
    #rho = 0.0740 #Air density at (75 F) (50% Humidity) (760 mm Hg pressure) (0 elevation)
    const = 0.005316 #5.316E-03 = 0.07182*rho*(5.125/mass)*(circ/9.125)^2
                     #Edit values for mass, circ, or rho to find new const if necessary
    Cd = 0.330 #Drag Coefficient
    #beta = 0.0001217 #Constant in calculating actual pressure not used in this script just for reference
    #SVP constant (Saturation Vapor Pressure)
    
  
    
    #Vin = mph*1.467
    #Vinz = Vin*math.sin(theta) #Takes vertical component of throw
    #Vinx = Vin*math.cos(theta)*math.sin(phi) #Takes horizontal component of throw then adjusts on horizontal plane. X axis = 1st base-third base line
    #Viny = Vin*math.cos(theta)*math.cos(phi)#Takes horizontal component of throw then adjusts on horizontal plane. Y axis = Home plate to pitching mound line
    Vinx = x[3]
    Viny = x[4]
    Vinz = x[5]
    Vin = math.sqrt(Vinx**2 + Viny**2 + Vinz**2)
    theta = math.asin(Vinz/Vin)
    #print(Viny/(Vin*math.cos(theta)))
    try:
        phi = math.acos(Viny/(Vin*math.cos(theta)))
    except ValueError:
        phi = math.pi
    #print(math.degrees(theta),math.degrees(phi))
    
    #theta = theta*(math.pi/180) #Convert to radians
    #phi = phi*(math.pi/180) #Convert to radians
    
    wx = (wb*math.cos(phi)-ws*math.sin(theta)*math.sin(phi)+wg*Vinx/Vin)*math.pi/30
    wy = (-wb*math.sin(phi)-ws*math.sin(theta)*math.cos(phi)+wg*Viny/Vin)*math.pi/30
    wz = (ws*math.cos(theta)+wg*Vinz/Vin)*math.pi/30
    
    temp = []
    temp.append(wx)
    temp.append(wy)
    temp.append(wz)
    angular.append(temp)
    
    w = math.sqrt(wx**2 + wy**2 + wz**2) #rad per sec
    rw =(baseballCircumference/(2*math.pi))*(w/12) #ft per sec
    
    vxw = 0 #Wind speed in x direction ft/sec
    vyw = 0 #Wind speed in y direction ft/sec
    
    
    #Re_100 = 210000 #2.100E+05 Reynolds number for 100 mph throw
    
    positionVector = []
    velocityVector = []
    accelerationVector = []
    
    initialPos = [x[0],x[1],x[2]]
    positionVector.append(initialPos)
    
    initialVel = [Vinx,Viny,Vinz]
    velocityVector.append(initialVel)
    
    initAccel = [x[6],x[7],x[8]]
    accelerationVector.append(initAccel)
    t = time[0]
    dt = dt
    tau = 10000 #Spin decay constant. Set large so spin doesn't decay much
        
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
    
    x = np.array([positionVector[-1][0],positionVector[-1][1],positionVector[-1][2], velocityVector[-1][0],velocityVector[-1][1],velocityVector[-1][2],accelerationVector[-1][0],accelerationVector[-1][1],accelerationVector[-1][2]])
    #print(x)
    #print(time[0])
    return(x)
    
'''Defines which of the UKF state variables are actually being measured (Camera measures position) (Spin is not being tracked by the Kalman filter)'''
def hx(x):
    return x[:3]


'''Unscented Kalman Filter Initialization'''
dt = 0.0001
#p = np.array([[5,0,0,0,0,0,0,0,0], [0,10,0,0,0,0,0,0,0],[0,0,5,0,0,0,0,0,0],[0,0,0,20,0,0,0,0,0,0],[0,0,0,0,40,0,0,0,0,0],[0,0,0,0,0,20,0,0,0],[0,0,0,0,0,0,15,0,0],[0,0,0,0,0,0,0,15,0],[0,0,0,0,0,0,0,0,20]])
points = MerweScaledSigmaPoints(n=9, alpha=.2, beta=2., kappa=-6.)
#sigmas = points.sigma_points(mean, p)
ukf = UnscentedKalmanFilter(dim_x=9, dim_z=3, dt=dt, hx=hx, fx=fx, points=points)
ukf.x = np.array([0.,55.,5,0.,-100.,0,0.,0.,-32])
ukf.P *= 1000.
ukf.R = np.diag([(0.25/3)**2,(0.25/3)**2,(0.25/3)**2])
ukf.Q = Q_discrete_white_noise(dim=3, dt=dt, var=0.0000001, block_size=3)

'''Spin Input Extraciton'''
spin = []
data = []

data, data_dict, spin, spin_dict, pitcher = get_json("SimulatedNoisyPitch.json")

print("Time after JSON: ", Time.time()-start)        


'''Separation of Position and Timestamp data'''
#z_std = 0.20
zs = [] # measurements
timeStamps = []
for i in data:
    
    element = [float(i[0]), float(i[1]),float(i[2])]
    zs.append(element)
    timeStamps.append(float(i[3]))
#print(zs)
#print(ukf.x)
#print(ukf.P)
#print(ukf.R)
#print(ukf.Q)

#(xs, ps) = ukf.batch_filter(zs)
    
angular = []

'''Initialization of Unscented Kalman Filter counter variables'''
counter = 0
xs = []
ps = []
#print(timeStamps)
Var = 0
time = [timeStamps[0]]

'''Unscented Kalman Filter Control Loop'''
for z in zs:
    timeStamp = timeStamps[counter]
    #aprint(timeStamp)
    #print("Predict")
    while(time[0] <= timeStamp):
        ukf.predict()
        time[0] += dt
        xs.append(ukf.x.copy())
        ps.append(ukf.P.copy())
        Var = Var+1
        #print(time[0])
    #print("update")
    print(Var)
    Var = 0
    ukf.R = np.diag([(0.1/3)**2,(((55-z[2])*0.01)/3)**2,(0.1/3)**2])
    #ukf.R = np.diag([(0.01/3)**2,(0.01/3)**2,(0.01/3)**2])
    #print(ukf.R)
    ukf.update(z)
    xs.append(ukf.x.copy())
    ps.append(ukf.P.copy())
    #print(ukf.x)
    #print(counter)
    counter+=1;
    
'''xs and ps serve as storage for the Kalman Filter state and confidence'''
xs = np.array(xs)
ps = np.array(ps)

'''Timer'''
print("Time after Filter: ", Time.time()-start)
#for z in zs:
#    ukf.predict()
#    ukf.update(z)
#    xs.append(ukf.x)
#    ps.append(ukf.P)
    #print(kf.x, 'log-likelihood', kf.log_likelihood)

'''
netDistance = 10
netPosition = 55-netDistance
netIndex = 0
#print(xs)
for i in range(0,len(xs)):
    if(xs[i][1] < netPosition):
        netIndex = i
        print(i)
        break
(sxs, Ps, K) = ukf.rts_smoother(xs[:(i+1)], ps[:(i+1)])

print("Time after Smoother: ", Time.time()-start)
'''

with open('KalmanFilterPosition.csv', 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerows(xs)
with open('KalmanFilterErrorDebug.csv', 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerows(ps[-1])
'''
with open('KalmanFilterPositionSmooth.csv', 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerows(sxs)
'''
with open('MeasuredPosition.csv', 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerows(zs)
with open('Angular.csv', 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerows(angular)
print("Time after CSV: ", Time.time()-start)