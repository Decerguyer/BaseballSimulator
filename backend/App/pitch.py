
from typing import List, Tuple, Dict
from vector3D import Vector3D
#from V3.UKFB_Routine import kalman_process
from V3.UKFB import UKFB
from V3.BaseballPhysicalModel import BaseballPhysicalModel

class Pitch:
    def __init__(self, up_dict: dict):
        #back end needs to wrap all prior fields into dict and pass into Pitch constructor
        self.up_dict = up_dict

        '''
        THIS IS THE CONTENTS OF THE UP_DICT
        self.positions = positions
        self.timestamps = timestamps
        self.spin = spin
        self.pitcher_id = pitcher_id
        self.serial_number = serial_number
        self.pitch_id = pitch_id
        self.error = error
        self.time = time
        self.mound_offset = mound_offset
        self.height_offset = height_offset
        '''

        #to be defined inside kalman_filter func
        self.p_dict = {}
        self.p_dict['pitch_id'] = self.up_dict['pitch_id']
        self.p_dict['serial_number'] = self.up_dict['serial_number']
        self.p_dict['user_id'] = self.up_dict['user_id']
        self.p_dict['time'] = self.up_dict['time']

    def to_unprocessed_dynamo_item(self):
        dynamo_positions = []
        dynamo_timestamps = []
        dynamo_error = []

        for position in self.up_dict.get('positions'):
            dynamo_positions.append(position.three_dim_to_dynamo_item())
        for err in self.up_dict.get('error'):
            dynamo_error.append(err.three_dim_to_dynamo_item())
        for timestamp in self.up_dict.get('timestamps'):
            dynamo_timestamps.append({'N': str(timestamp)})

        return {
            'positions': {'L': dynamo_positions},
            'timestamps': {'L': dynamo_timestamps},
            #ideal spin implementation, pitch logic uses wb, ws, wg for time being
            'spin': self.up_dict['spin'].three_dim_to_dynamo_item(),
            'error': {'L': dynamo_error},
            'serial_number': {'N': str(self.up_dict.get('serial_number'))},
            'user_id': {'S': str(self.up_dict.get('user_id'))},
            'pitch_id': {'S': str(self.up_dict.get('pitch_id'))},
            'time': {'S': str(self.up_dict.get('time'))},
            'mound_offset': {'N': str(self.up_dict.get('mound_offset'))},
            'height_offset': {'N': str(self.up_dict.get('height_offset'))}
        }

    def to_processed_dynamo_item(self):
        dynamo_positions = []
        dynamo_rts_positions = []

        for position in self.p_dict.get('positions'):
            dynamo_positions.append(position.three_dim_to_dynamo_item())
        for rts_position in self.p_dict.get('rts_positions'):
            dynamo_rts_positions.append(rts_position.three_dim_to_dynamo_item())
        return {
            'positions': {'L': dynamo_positions},
            'rts_positions': {'L': dynamo_rts_positions},
            'serial_number': {'N': str(self.p_dict.get('serial_number'))},
            'user_id': {'S': str(self.p_dict.get('user_id'))},
            'pitch_id': {'S': str(self.p_dict.get('pitch_id'))},
            'time': {'S': str(self.p_dict.get('time'))}
        }

    def kalman_filter(self):
        UKF = UKFB(self.up_dict)
        UKF.control_loop()
        kalman_output = UKF.output_dict()

        PM = BaseballPhysicalModel(kalman_output)
        PM.physical_model_control()
        PM.print_output()

        downSampled = []
        samples = 20
        step = int(len(PM.positionVector) / samples)
        for i in range(0, len(PM.positionVector), step):
            temp = Vector3D(round(PM.positionVector[i][0], 2), round(PM.positionVector[i][1], 2),
                            round(PM.positionVector[i][2], 2))
            downSampled.append(temp)
        if (len(PM.positionVector) - 1) % step != 0:
            temp = Vector3D(round(PM.positionVector[-1][0], 2), round(PM.positionVector[-1][1], 2),
                            round(PM.positionVector[-1][2], 2))
            downSampled.append(temp)

        smoothed = UKF.position_smoother()
        samples = 8
        step = int(len(smoothed) / samples)
        if step < 1:
            step = 1
        # print (step)
        downSampledRTS = []
        # len(smoothed-1) because the last rts_position is the start of the phys model positions
        # No need to include it twice
        for i in range(0, len(smoothed)-1, step):
            temp = Vector3D(round(smoothed[i][0], 2), round(smoothed[i][1], 2), round(smoothed[i][2], 2))
            downSampledRTS.append(temp)

        self.p_dict["positions"] = downSampled
        self.p_dict["rts_positions"] = downSampledRTS
