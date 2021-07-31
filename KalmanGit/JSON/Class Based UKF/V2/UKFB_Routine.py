from HTTPHandler import HTTPHandler
from UKFB import UKFB
from BaseballPhysicalModel import BaseballPhysicalModel
from JSONControl import JSONControl


mount_offset = float(input("Enter the distance from the mount to the Target in feet: "))
height_offset = float(input("Enter the height of the inner top right corner relative to the ground: "))

http_handler = HTTPHandler("JEYSolutions")
json_data = http_handler.get_most_recent_pitch()

print(json_data)
#print(json_data["positions"])

UKF = UKFB(json_data, mount_offset, height_offset)
UKF.control_loop()
kalman_output = UKF.output_dict()

PM = BaseballPhysicalModel(kalman_output)
PM.physical_model_control()
PM.print_output()

json_manager = JSONControl()
json_manager.write_output_json("OutputFile.json", PM.positionVector)

'''
count = 0
downSampled = []
for i in PM.positionVector:
    empty = []
    if count % 50 == 0:
        x = round(i[0], 2)
        y = round(i[1], 2)
        z = round(i[2], 2)
        empty.append(x)
        empty.append(y)
        empty.append(z)
        downSampled.append(empty)
    count += 1
if(len(PM.positionVector)-1)%50 !=0 :
    x = round(PM.positionVector[-1][0], 2)
    y = round(PM.positionVector[-1][1], 2)
    z = round(PM.positionVector[-1][2], 2)
    temp = []
    temp.append(x)
    temp.append(y)
    temp.append(z)
    downSampled.append(temp)

json_manager.write_output_json("DownSampled.json", downSampled)
'''