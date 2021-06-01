# UKFB.py
# Created 5/7/21
# By Yoni Arieh

# This Class is a specific Unscented Kalman Filter for a baseball trajectory with position/spin measurements
# The filter only adjusts error using the position measurements, the spin is used as given

# Input: File in -> Dictionary out
# Output: Dictionary in -> File out



import math
from filterpy.kalman import UnscentedKalmanFilter
from filterpy.kalman import unscented_transform, MerweScaledSigmaPoints
from filterpy.common import Q_discrete_white_noise
import numpy as np
from numpy.random import randn


class UKFB:

    # Constructor, takes file path
    def __init__(self, input_dict, mound_offset):

        # FilterPy UKF class initializations
        self.dt = 0.0001
        self.ukf = UnscentedKalmanFilter(dim_x=9, dim_z=3, dt=self.dt, hx=self.hx, fx=self.fx,
                                         points=MerweScaledSigmaPoints(n=9, alpha=.2, beta=2., kappa=-6.))
        self.ukf.x = np.array([0., 60., 5, 0., -100., 0, 0., 0., -32])
        self.ukf.P *= 1000.
        self.ukf.R = np.diag([(0.25 / 3) ** 2, (0.25 / 3) ** 2, (0.25 / 3) ** 2])
        self.ukf.Q = Q_discrete_white_noise(dim=3, dt=self.dt, var=0.0000001, block_size=3)

        # Input Dictionary declarations
        self.zs, self.timeStamps, self.spin, self.error = self.json_manager(input_dict, mound_offset)

        # UKFB Control Loop Variable Declarations
        self.counter = 0
        self.xs = []
        self.ps = []
        # print(timeStamps)
        self.var = 0
        self.time = [self.timeStamps[0]]

    '''State Transition function for UKF (Determines the physical model)'''
    # x = state, dt = dt, time = array with time value (1 element array only needs current value)
    # Assumes acc, vel and pos in state are in ft/s**2, ft/s and ft
    # Assumes spin components are in RPM and remain constant throughout

    def fx(self, x, dt):  # Generalized State transition function

        wb = self.spin[0]
        ws = self.spin[1]
        wg = self.spin[2]

        # baseballMass = 5.125 #Oz
        baseballCircumference = 9.125  # inches
        # rho = 0.0740 #Air density at (75 F) (50% Humidity) (760 mm Hg pressure) (0 elevation)
        const = 0.005316  # 5.316E-03 = 0.07182*rho*(5.125/mass)*(circ/9.125)^2
        # Edit values for mass, circ, or rho to find new const if necessary
        Cd = 0.330  # Drag Coefficient
        # beta = 0.0001217 #Constant in calculating actual pressure not used in this script just for reference
        # SVP constant (Saturation Vapor Pressure)

        # Vin = mph*1.467
        # Vinz = Vin*math.sin(theta) #Takes vertical component of throw
        # Vinx = Vin*math.cos(theta)*math.sin(phi) #Takes horizontal component of throw then adjusts on horizontal plane. X axis = 1st base-third base line
        # Viny = Vin*math.cos(theta)*math.cos(phi)#Takes horizontal component of throw then adjusts on horizontal plane. Y axis = Home plate to pitching mound line
        Vinx = x[3]
        Viny = x[4]
        Vinz = x[5]
        Vin = math.sqrt(Vinx ** 2 + Viny ** 2 + Vinz ** 2)
        theta = math.asin(Vinz / Vin)
        # print(Viny/(Vin*math.cos(theta)))
        try:
            phi = math.acos(Viny / (Vin * math.cos(theta)))
        except ValueError:
            phi = math.pi
        # print(math.degrees(theta),math.degrees(phi))

        # theta = theta*(math.pi/180) #Convert to radians
        # phi = phi*(math.pi/180) #Convert to radians

        wx = (wb * math.cos(phi) - ws * math.sin(theta) * math.sin(phi) + wg * Vinx / Vin) * math.pi / 30
        wy = (-wb * math.sin(phi) - ws * math.sin(theta) * math.cos(phi) + wg * Viny / Vin) * math.pi / 30
        wz = (ws * math.cos(theta) + wg * Vinz / Vin) * math.pi / 30

        temp = []
        temp.append(wx)
        temp.append(wy)
        temp.append(wz)
        # angular.append(temp)

        w = math.sqrt(wx ** 2 + wy ** 2 + wz ** 2)  # rad per sec
        rw = (baseballCircumference / (2 * math.pi)) * (w / 12)  # ft per sec

        vxw = 0  # Wind speed in x direction ft/sec
        vyw = 0  # Wind speed in y direction ft/sec

        # Re_100 = 210000 #2.100E+05 Reynolds number for 100 mph throw

        positionVector = []
        velocityVector = []
        accelerationVector = []

        initialPos = [x[0], x[1], x[2]]
        positionVector.append(initialPos)

        initialVel = [Vinx, Viny, Vinz]
        velocityVector.append(initialVel)

        initAccel = [x[6], x[7], x[8]]
        accelerationVector.append(initAccel)
        t = self.time[0]
        dt = self.dt
        tau = 10000  # Spin decay constant. Set large so spin doesn't decay much

        vw = math.sqrt((velocityVector[-1][0] - vxw) ** 2 + (velocityVector[-1][1] - vyw) ** 2 + (
        velocityVector[-1][2]) ** 2)  # Wind adjusted Velocity

        aDragx = const * (velocityVector[-1][0] - vxw) * Cd * vw * -1
        aDragy = const * (velocityVector[-1][1] - vyw) * Cd * vw * -1
        aDragz = const * (velocityVector[-1][2]) * Cd * vw * -1

        S = (rw / vw) * (math.e ** ((t * -1) / tau))
        Cl = 1.0 / (2.32 + (0.4 / S))
        aMagx = const * vw * (wy * velocityVector[-1][2] - wz * (velocityVector[-1][1] - vyw)) * (Cl / w)
        aMagy = const * vw * (wz * (velocityVector[-1][0] - vxw) - wx * (velocityVector[-1][2])) * (Cl / w)
        aMagz = const * vw * (wx * (velocityVector[-1][1] - vyw) - wy * (velocityVector[-1][0])) * (Cl / w)

        aX = aDragx + aMagx
        aY = aDragy + aMagy
        aZ = aDragz + aMagz - 32.174  # Gravity
        acceleration = [aX, aY, aZ]
        accelerationVector.append(acceleration)

        vX = velocityVector[-1][0] + aX * dt
        vY = velocityVector[-1][1] + aY * dt
        vZ = velocityVector[-1][2] + aZ * dt
        velocity = [vX, vY, vZ]
        velocityVector.append(velocity)

        pX = positionVector[-1][0] + vX * dt + 0.5 * aX * dt * dt
        pY = positionVector[-1][1] + vY * dt + 0.5 * aY * dt * dt
        pZ = positionVector[-1][2] + vZ * dt + 0.5 * aZ * dt * dt
        position = [pX, pY, pZ]
        positionVector.append(position)

        x = np.array([positionVector[-1][0], positionVector[-1][1], positionVector[-1][2], velocityVector[-1][0],
                      velocityVector[-1][1], velocityVector[-1][2], accelerationVector[-1][0], accelerationVector[-1][1],
                      accelerationVector[-1][2]])
        # print(x)
        # print(time[0])
        return x

    @staticmethod
    def hx(x):
        """
        :param x: State np.array that contains x,y,z,Vx,Vy,Vz,Ax,Ay,Az
        :type x: numpy.array
        """
        return x[:3]

    def control_loop(self):
        for z in self.zs:
            time_stamp = self.timeStamps[self.counter]
            # aprint(timeStamp)
            print("Predict")
            while self.time[0] <= time_stamp:
                self.ukf.predict()
                self.time[0] += self.dt
                self.xs.append(self.ukf.x.copy())
                self.ps.append(self.ukf.P.copy())
                self.var = self.var + 1
                # print(time[0])
            # print("update")
            print(self.var)
            self.var = 0
            self.ukf.R = np.diag([(self.error[self.counter][0] / 3) ** 2, (self.error[self.counter][1] / 3) ** 2, (self.error[self.counter][2]/3) ** 2])
            self.ukf.update(z)
            print("Updated")
            self.xs.append(self.ukf.x.copy())
            self.ps.append(self.ukf.P.copy())
            self.counter += 1

    @staticmethod
    def json_manager(input_dict: dict, mound_offset):
        """
        :param input_dict: The JSON Dictionary returned from the Generic JSON Control class
        :type input_dict: dict
        :param mound_offset: distance from mound to target
        :type mound_offset: float
        """
        position_dictionary = input_dict["positions"]
        spin_dictionary = input_dict["spin"]
        error_dictionary = input_dict["error"]
        pitcher = input_dict["pitcher_id"]
        time_stamp_dict = input_dict["timestamps"]

        zs = []
        for position in position_dictionary:
            #Z becomes Y and is inverted. Y becomes Z and is inverted. X is inverted
            #Convert meters to feet
            #Add 60-mound_offset to the Y value (was Z)
            offset = 60-mound_offset
            position_set = [(position[0]*-1)/0.3048, (position[2]*-1)/0.3048+offset, (position[1]*-1)/0.3048]
            zs.append(position_set)

        r = []
        for error in error_dictionary:
            # Z becomes Y and is inverted. Y becomes Z and is inverted. X is inverted
            # Convert meters to feet
            error_set = [(error[0]*-1)/0.3048, (error[2]*-1)/0.3048, (error[1]*-1)/0.3048]
            r.append(error_set)

        time_stamps = []
        first_time_stamp = time_stamp_dict[0]
        for timestamp in time_stamp_dict:
            #Offset all the timestamps to the first
            time_stamps.append((timestamp-first_time_stamp)/1000000)

        spin = [spin_dictionary[0], spin_dictionary[1], spin_dictionary[2]]

        print(zs)
        print(time_stamps)
        print(spin)
        print(r)

        return zs, time_stamps, spin, r

    def print_output(self):
        print(self.xs[-1])

    def output_dict(self):
        state = {"x": self.xs[-1][0], "y": self.xs[-1][1], "z": self.xs[-1][2],
                 "Vx": self.xs[-1][3], "Vy": self.xs[-1][4], "Vz": self.xs[-1][5]}
        spin = {"x": self.spin[0], "y": self.spin[1], "z": self.spin[2]}
        output = {"state": state, "spin": spin}
        return output
