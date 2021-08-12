from HTTPHandler import HTTPHandler
from UKFB import UKFB
from BaseballPhysicalModel import BaseballPhysicalModel
from JSONControl import JSONControl


mount_offset = float(input("Enter the distance from the mount to the Target in feet: "))
height_offset = float(input("Enter the height of the inner top right corner relative to the ground: "))

http_handler = HTTPHandler("JEYSolutions")
json_data = http_handler.get_most_recent_pitch()
json_data["mound_offset"] = mount_offset
json_data["height_offset"] = height_offset
print(json_data)
#print(json_data["positions"])

UKF = UKFB(json_data)
UKF.control_loop()
kalman_output = UKF.output_dict()

PM = BaseballPhysicalModel(kalman_output)
PM.physical_model_control()
PM.print_output()

json_manager = JSONControl()
json_manager.write_output_json("OutputFile.json", PM.positionVector)


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

json_manager.write_output_json("DownSampled.json", downSampled)

json_manager.write_output_json("InputData.json", json_data)

smoothed = UKF.position_smoother()
json_manager.write_output_json("Smoothed.json", smoothed)