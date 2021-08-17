import math
import csv


def baseballTrajectory(Xi, Yi, Zi, Vinx, Viny, Vinz, wb, ws, wg):
    print(Xi, Yi, Zi, Vinx, Viny, Vinz, wb, ws, wg)
    baseballMass = 5.125  # Oz
    baseballCircumference = 9.125  # inches
    # Air density at (75 F) (50% Humidity) (760 mm Hg pressure) (0 elevation)
    rho = 0.0740
    const = 0.005316  # 5.316E-03 = 0.07182*rho*(5.125/mass)*(circ/9.125)^2
    # Edit values for mass, circ, or rho to find new const if necessary
    Cd = 0.330  # Drag Coefficient
    # beta = 0.0001217 #Constant in calculating actual pressure not used in this script just for reference
    # SVP constant (Saturation Vapor Pressure)
    # theta = theta*(math.pi/180) #Convert to radians
    # phi = phi*(math.pi/180) #Convert to radians

    #Vin = mph*1.467
    # Vinz = Vin*math.sin(theta) #Takes vertical component of throw
    # Vinx = Vin*math.cos(theta)*math.sin(phi) #Takes horizontal component of throw then adjusts on horizontal plane. X axis = 1st base-third base line
    # Viny = Vin*math.cos(theta)*math.cos(phi)#Takes horizontal component of throw then adjusts on horizontal plane. Y axis = Home plate to pitching mound line
    Vin = math.sqrt(Vinx**2 + Viny**2 + Vinz**2)
    theta = math.asin(Vinz/Vin)
    phi = math.acos(Viny/(Vin*math.cos(theta)))
    print(math.degrees(theta), math.degrees(phi))

    wx = (wb*math.cos(phi)-ws*math.sin(theta) *
          math.sin(phi)+wg*Vinx/Vin)*math.pi/30
    wy = (-wb*math.sin(phi)-ws*math.sin(theta)
          * math.cos(phi)+wg*Viny/Vin)*math.pi/30
    wz = (ws*math.cos(theta)+wg*Vinz/Vin)*math.pi/30

    w = math.sqrt(wx**2 + wy**2 + wz**2)  # rad per sec
    rw = (baseballCircumference/(2*math.pi))*(w/12)  # ft per sec

    vxw = 0  # Wind speed in x direction ft/sec
    vyw = 0  # Wind speed in y direction ft/sec

    # Re_100 = 210000 #2.100E+05 Reynolds number for 100 mph throw

    positionVector = []
    velocityVector = []
    accelerationVector = []

    initialPos = [Xi, Yi, Zi]
    positionVector.append(initialPos)

    initialVel = [Vinx, Viny, Vinz]
    velocityVector.append(initialVel)
    t = 0.
    dt = 0.0001
    tau = 10000  # Spin decay constant. Set large so spin doesn't decay much
    # Run trajectory while the ball is front of home plate and the ball is 4.8 inches above the ground
    while(positionVector[-1][1] > 1.4 and positionVector[-1][2] > 0.4):

        vw = math.sqrt((velocityVector[-1][0]-vxw)**2 + (velocityVector[-1][1]-vyw)**2 + (
            velocityVector[-1][2])**2)  # Wind adjusted Velocity

        aDragx = const*(velocityVector[-1][0]-vxw)*Cd*vw*-1
        aDragy = const*(velocityVector[-1][1]-vyw)*Cd*vw*-1
        aDragz = const*(velocityVector[-1][2])*Cd*vw*-1

        S = (rw/vw)*(math.e**((t*-1)/tau))
        Cl = 1.0/(2.32+(0.4/S))
        aMagx = const*vw * \
            (wy*velocityVector[-1][2]-wz*(velocityVector[-1][1]-vyw))*(Cl/w)
        aMagy = const*vw * \
            (wz*(velocityVector[-1][0]-vxw)-wx*(velocityVector[-1][2]))*(Cl/w)
        aMagz = const*vw * \
            (wx*(velocityVector[-1][1]-vyw)-wy*(velocityVector[-1][0]))*(Cl/w)

        aX = aDragx+aMagx
        aY = aDragy+aMagy
        aZ = aDragz+aMagz - 32.174  # Gravity
        acceleration = [aX, aY, aZ]
        accelerationVector.append(acceleration)

        vX = velocityVector[-1][0] + aX*dt
        vY = velocityVector[-1][1] + aY*dt
        vZ = velocityVector[-1][2] + aZ*dt
        velocity = [vX, vY, vZ]
        velocityVector.append(velocity)

        pX = positionVector[-1][0] + vX*dt + 0.5*aX*dt*dt
        pY = positionVector[-1][1] + vY*dt + 0.5*aY*dt*dt
        pZ = positionVector[-1][2] + vZ*dt + 0.5*aZ*dt*dt
        position = [pX, pY, pZ]
        positionVector.append(position)

        t += dt  # Increment time
    # print(positionVector)
    with open('FullSystemSimulation.csv', 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerows(positionVector)
        # writer.writerows(velocityVector)
        # writer.writerows(accelerationVector)
