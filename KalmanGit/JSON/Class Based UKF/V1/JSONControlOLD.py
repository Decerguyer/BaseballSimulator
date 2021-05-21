import json


class JSONControl:

    # Constructor, takes file path
    #def __init__(self):

    # called on ParseJSON with no input
    # data_array: returns an array of the positions
    # data_dictionary: dictionary of positions
    # spin: array of spin values
    # spin_dictionary: dictionary of the spin values and keys
    # pitcher: string that represents pitcher name pitcher
    @staticmethod
    def get_kalman_input_json(file_name):
        with open(file_name) as json_file:
            json_data = json.load(json_file)

        spin_dictionary = json_data["spin"]
        pitcher = json_data["pitcherId"]
        data_dictionary = json_data["positions"]
        data_array = []
        for position in data_dictionary:
            sub_arr = [position["x"], position["y"], position["z"], position["timestamp"]]
            data_array.append(sub_arr)

        spin = [spin_dictionary["x"], spin_dictionary["y"], spin_dictionary["z"]]
        return data_array, data_dictionary, spin, spin_dictionary, pitcher

    # inputs: file_name: string representing file name (Will be related to pitch serial #)
    #         state_dict: all floats {"x":x,"y":y,"z":z,"Vx":Vx,"Vy":Vy,"Vz":Vz}
    #         spin_dict: {"x":float x,"y":float y,"z":float z}
    @staticmethod
    def write_kalman_output_json(file_name, state_dict, spin_dict):
        """
        :param file_name: the name of the file to be outputted
        :type file_name: str
        :param state_dict: dictionary of x, y, z, Vx, Vy, Vz
        :type state_dict: dict
        :param spin_dict: dictionary of spin vals x, y, z
        :type spin_dict: dict
        """
        final_json = {"state": state_dict, "spin": spin_dict}
        with open(file_name, 'w') as output_file:
            json.dump(final_json, output_file)

    @staticmethod
    def get_physical_model_input_json(file_name):
        with open(file_name) as json_file:
            json_data = json.load(json_file)

        state_dict = json_data["state"]
        spin_dict = json_data["spin"]

        return state_dict, spin_dict

    @staticmethod
    def write_physical_model_output_json(file_name, position_arr):
        """
        :param file_name: the name of the file to be outputted
        :type file_name: str
        :param position_arr: dictionary of x, y, z, timestamp
        :type position_arr: list
        """
        final_json = {"positions": position_arr}
        with open(file_name, 'w') as output_file:
            json.dump(final_json, output_file)

    # functions for debugging (Writes simulate JSON)
    @staticmethod
    def write_simulated_json(file_name, positions, spin, pitcher):

        # convert array of array positions to [{}]
        pos_dict_array = []
        for pos in positions:
            cur_dict = {"x": pos[0], "y": pos[1], "z": pos[2]}
            pos_dict_array.append(cur_dict)
        final_json = {"positions": pos_dict_array, "spin": spin, "pitcherId": pitcher}
        with open(file_name, 'w') as output_file:
            json.dump(final_json, output_file)

    # file_name -> string for file name
    # noisyPositions -> [[x, y, z, timestamp], [x, y, z, timestamp], ...]
    # spin -> {x: float, y: float, z: float}
    # pitcher -> string of pitcher name
    @staticmethod
    def write_simulated_noisy_json(file_name, noisyPositions, spin, pitcher):
        # convert array of array positions to [{}]
        pos_dict_array = []
        for pos in noisyPositions:
            cur_dict = {"x": pos[0], "y": pos[1], "z": pos[2], "timestamp": pos[3]}
            pos_dict_array.append(cur_dict)
        final_json = {"positions": pos_dict_array, "spin": spin, "pitcherId": pitcher}
        with open(file_name, 'w') as output_file:
            json.dump(final_json, output_file)
