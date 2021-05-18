from JSONControl import JSONControl
from UKFB import UKFB
from BaseballPhysicalModel import BaseballPhysicalModel

json_manager = JSONControl()
json_data = json_manager.get_input_json("SimulatedNoisyPitch.json")

UKF = UKFB(json_data)
UKF.control_loop()
kalman_output = UKF.output_dict()

PM = BaseballPhysicalModel(kalman_output)
PM.physical_model_control()
PM.print_output()
json_manager.write_output_json("OutputFile.json", PM.positionVector)
