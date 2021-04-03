#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Jul 13 18:55:21 2020

@author: yonatanarieh
"""

from mpl_toolkits import mplot3d
#%matplotlib inline
import numpy as np
import matplotlib.pyplot as plt
import csv

fig = plt.figure()
ax = plt.axes(projection='3d')
finalPos = []

xdata = []
ydata = []
zdata = []

with open('TrajectoryDataPosition copy.csv', newline='') as f:
    reader = csv.reader(f)
    for row in reader:
        xdata.append(float(row[0]))
        ydata.append(float(row[1]))
        zdata.append(float(row[2]))
        #print(xdata[-1],ydata[-1],zdata[-1])
finalPos.append(xdata[-2])
finalPos.append(ydata[-2])
finalPos.append(zdata[-2])
ax.scatter3D(xdata, ydata, zdata, c=zdata, cmap='Greens');

xdata = []
ydata = []
zdata = []

with open('MeasuredPosition.csv', newline='') as f:
    reader = csv.reader(f)
    for row in reader:
        xdata.append(float(row[0]))
        ydata.append(float(row[1]))
        zdata.append(float(row[2]))
        #print(xdata[-1],ydata[-1],zdata[-1])
        
ax.scatter3D(xdata, ydata, zdata, c=zdata, cmap='Reds');

xdata = []
ydata = []
zdata = []


with open('KalmanFilterPositionSmooth.csv', newline='') as f:
    reader = csv.reader(f)
    for row in reader:
        xdata.append(float(row[0]))
        ydata.append(float(row[1]))
        zdata.append(float(row[2]))
        
ax.scatter3D(xdata, ydata, zdata, c=zdata, cmap='Blues');

xdata = []
ydata = []
zdata = []

with open('FullSystemSimulation.csv', newline='') as f:
    reader = csv.reader(f)
    for row in reader:
        xdata.append(float(row[0]))
        ydata.append(float(row[1]))
        zdata.append(float(row[2]))
        
ax.scatter3D(xdata, ydata, zdata, c=zdata, cmap='Oranges');

print("Error in x (inches): ", abs(xdata[-2]-finalPos[0])*12)
print("Error in y (inches): ", abs(ydata[-2]-finalPos[1])*12)
print("Error in z (inches): ", abs(zdata[-2]-finalPos[2])*12)
print(xdata[-2])
print(zdata[-2])
