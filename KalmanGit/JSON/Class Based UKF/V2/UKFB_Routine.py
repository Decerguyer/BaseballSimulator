from HTTPHandler import HTTPHandler
from UKFB import UKFB
from BaseballPhysicalModel import BaseballPhysicalModel
from JSONControl import JSONControl

http_handler = HTTPHandler("owen gay")
json_data = http_handler.get_most_recent_pitch()

print(json_data)
print(json_data["positions"])
'''
UKF = UKFB(json_data)
UKF.control_loop()
kalman_output = UKF.output_dict()

PM = BaseballPhysicalModel(kalman_output)
PM.physical_model_control()
PM.print_output()

json_manager = JSONControl()
json_manager.write_output_json("OutputFile.json", PM.positionVector)
'''