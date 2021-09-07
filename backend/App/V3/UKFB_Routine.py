#from HTTPHandler import HTTPHandler
from UKFB import UKFB
from BaseballPhysicalModel import BaseballPhysicalModel
#from JSONControl import JSONControl

def kalman_process(json_data : dict):
    UKF = UKFB(json_data)
    UKF.control_loop()
    kalman_output = UKF.output_dict()

    PM = BaseballPhysicalModel(kalman_output)
    PM.physical_model_control()
    PM.print_output()

    downSampled = []
    samples = 20
    step = int(len(PM.positionVector)/samples)
    for i in range(0, len(PM.positionVector), step):
        x = round(PM.positionVector[i][0], 2)
        y = round(PM.positionVector[i][1], 2)
        z = round(PM.positionVector[i][2], 2)
        empty = [x, y, z]
        downSampled.append(empty)
    if(len(PM.positionVector)-1) % step != 0:
        x = round(PM.positionVector[-1][0], 2)
        y = round(PM.positionVector[-1][1], 2)
        z = round(PM.positionVector[-1][2], 2)
        temp = [x, y, z]
        downSampled.append(temp)

    smoothed = UKF.position_smoother()
    kalman_output["rts_positions"] = smoothed
    kalman_output["positions"] = PM.positionVector

    return kalman_output