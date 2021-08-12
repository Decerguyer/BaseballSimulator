
from typing import List, Tuple, Dict
from vector3D import Vector3D


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
            #'error': {'L': dynamo_error},
            'serial_number': {'N': str(self.up_dict.get('serial_number'))},
            'user_id': {'S': str(self.up_dict.get('user_id'))},
            'pitch_id': {'S': str(self.up_dict.get('pitch_id'))},
            'time': {'S': str(self.up_dict.get('time'))},
            'mound_offset': {'N': str(self.up_dict.get('mound_offset'))},
            'height_offset': {'N': str(self.up_dict.get('height_offset'))}
        }

    def to_processed_dynamo_item(self, ):
        dynamo_positions = []
        dynamo_rts_positions = []

        #for position in self.p_dict.get('positions'):
        #    dynamo_positions.append(position.three_dim_to_dynamo_item())
        #for rts_position in self.p_dict.get('rts_positions'):
        #    dynamo_positions.append(rts_position.three_dim_to_dynamo_item())

        return {
        #    'positions': {'L': dynamo_positions},
        #    'rts_positions': {'L': dynamo_rts_positions},
            'serial_number': {'N': str(self.p_dict.get('serial_number'))},
            'user_id': {'S': str(self.p_dict.get('user_id'))},
            'pitch_id': {'S': str(self.p_dict.get('pitch_id'))},
            'time': {'S': str(self.p_dict.get('time'))},
            #'mound_offset': {'N': str(self.p_dict.get('mound_offset'))},
            #'height_offset': {'N': str(self.p_dict.get('height_offset'))}
        }

    def kalman_filter(self):
        print('filter the pitch')

        # pass input_dict into kalman filter
        # smooth result
        # set result equal to p_dict

    def simulate(self):
        print('simulate the rest of the pitch from the kalman filter')

    def smooth(self):
        print("smooth the 'tracked' portion of the pitch")

    def routine(self):
        self.kalman_filter()
        self.simulate()
        self.smooth()