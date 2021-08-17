# JSONControl.py
# Created 5/7/21
# By Yoni Arieh

# This Class is a generic JSON controller

# Input: File in -> Dictionary out
# Output: Dictionary in -> File out

# Input files are read into a dictionary format and returned
# Output files are made by writing a dictionary to the JSON file
# It is the job of the main process to format the data into a single dictionary to be written to a JSON

import json


class JSONControl:

    # Constructor, takes file path
    # def __init__(self):

    # called on ParseJSON with no input
    # data_array: returns an array of the positions
    # data_dictionary: dictionary of positions
    # spin: array of spin values
    # spin_dictionary: dictionary of the spin values and keys
    # pitcher: string that represents pitcher name pitcher
    @staticmethod
    def get_input_json(file_name):
        """
        :param file_name: the name of the file to be Input
        :type file_name: str
        """
        with open(file_name) as json_file:
            json_data = json.load(json_file)
        return json_data

    # inputs: file_name: string representing file name (Will be related to pitch serial #)
    #         state_dict: all floats {"x":x,"y":y,"z":z,"Vx":Vx,"Vy":Vy,"Vz":Vz}
    #         spin_dict: {"x":float x,"y":float y,"z":float z}
    @staticmethod
    def write_output_json(file_name, output_dict):
        """
        :param file_name: the name of the file to be outputted
        :type file_name: str
        :param output_dict: dictionary of to be written to JSON file format
        :type output_dict: dict
        """

        with open(file_name, 'w') as output_file:
            json.dump(output_dict, output_file)