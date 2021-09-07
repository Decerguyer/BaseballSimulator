class SysConfig:
    def __init__(self, config_dict: dict):
        self.config_dict = config_dict

    def to_processed_dynamo_item(self):
        return {
            'serial_number': {'N': str(self.config_dict.get('serial_number'))},
            'user_id': {'S': str(self.config_dict.get('user_id'))},
            'mound_offset': {'N': str(self.config_dict.get('mound_offset'))},
            'height_offset': {'N': str(self.config_dict.get('height_offset'))}
        }
