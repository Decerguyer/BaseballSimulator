from flask import Flask, jsonify, request
from typing import List, Tuple, Dict
import os
import boto3
import math
import datetime
import uuid
import json
from flask_cors import CORS

from utils import convert_pitch_to_response, get_user_from_serial, get_mound_offset_from_serial, get_height_offset_from_serial
from pitch import Pitch
from vector3D import Vector3D

app = Flask(__name__)
CORS(app)

UNPROCESSED_PITCHES_TABLE = os.environ.get('UNPROCESSED_PITCHES_TABLE')
PROCESSED_PITCHES_TABLE = os.environ.get('PROCESSED_PITCHES_TABLE')
IS_OFFLINE = os.environ.get('IS_OFFLINE')

if IS_OFFLINE:
    client = boto3.client(
        'dynamodb',
        region_name='localhost',
        endpoint_url='http://localhost:8000'
    )
else:
    client = boto3.client('dynamodb')


@app.route("/")
def hello():
    return jsonify("This is the backend API for the baseball simulator app")


@app.route("/pitch/<string:pitch_id>")
def get_user(pitch_id):
    try:
        resp = client.get_item(
            TableName=PROCESSED_PITCHES_TABLE,
            Key={
                'pitch_id': {'S': pitch_id}
            }
        )
        pitch = resp.get('Item')
        if not pitch:
            return jsonify({'error': 'Pitch does not exist'}), 404

        return jsonify(convert_pitch_to_response(pitch)), 200
    except Exception as e:
        return jsonify({'error': str(e)}), 400


@app.route("/pitch", methods=["GET"])
def get_pitch_history():
    try:
        resp = client.scan(
            TableName=PROCESSED_PITCHES_TABLE
        )
        result = []
        print('Response: ', resp)
        for pitch in resp.get('Items'):
            result.append(convert_pitch_to_response(pitch))
        return jsonify(result), 200
    except Exception as e:
        return jsonify({'error': str(e)}), 400


@app.route("/pitch", methods=["POST"])
def record_pitch():
    try:

        up_dict = {}
        positions = []
        for position in request.json.get('positions'):
            positions.append(Vector3D(*position))
        up_dict['positions'] = positions
        up_dict['spin'] = Vector3D(*request.json.get('spin'))
        errors = []
        for error in request.json.get('error'):
            errors.append(Vector3D(*error))
        up_dict['error'] = errors
        up_dict['timestamps'] = request.json.get('timestamps')
        up_dict['serial_number'] = request.json.get('serial_number')

        if not up_dict['positions'] or not up_dict['spin'] or not up_dict['error'] \
                or not up_dict['timestamps'] or not up_dict['serial_number']:
            return jsonify({'error': 'Please provide valid positions, spin and error'}), 400

        up_dict['time'] = str(datetime.datetime.now())
        up_dict['pitch_id'] = uuid.uuid4().hex

        # get user_id from config dynamodb table
        #Also mound and height offsets
        # NEEDS TO BE IMPLEMENTED
        up_dict['user_id'] = get_user_from_serial(up_dict['serial_number'])
        up_dict['mound_offset'] = get_mound_offset_from_serial(up_dict['serial_number'])
        up_dict['height_offset'] = get_height_offset_from_serial(up_dict['serial_number'])


        # RELEVANT TO KF: positions, timestamps, spin, error_list
        pitch = Pitch(up_dict)
        pitch.routine()

        # post to unprocessed table first
        up_resp = client.put_item(
            TableName=UNPROCESSED_PITCHES_TABLE,
            Item=pitch.to_unprocessed_dynamo_item()
        )

        # post to processed table second
        p_resp = client.put_item(
            TableName=PROCESSED_PITCHES_TABLE,
            Item=pitch.to_processed_dynamo_item()
        )

        return jsonify({
            'success': 'recorded pitch with id: {}'.format(up_dict['pitch_id'])
        }), 201
    except Exception as e:
        return jsonify({'error': str(e)}), 400

def convert_pitch_to_response(pitch):
    positions = []
    timestamps = []
    errors = []
    spin = []
    for position in pitch.get('positions').get('L'):
        positions.append([float(position.get('M')['x'].get('N')), float(position.get('M')['y'].get('N')),
                          float(position.get('M')['z'].get('N'))])
    for error in pitch.get('error').get('L'):
        errors.append([float(error.get('M')['x'].get('N')), float(error.get('M')['y'].get('N')),
                       float(error.get('M')['z'].get('N'))])
    for timestamp in pitch.get('timestamps').get('L'):
        timestamps.append(float(timestamp.get('N')))
    spin = [float(pitch.get('spin').get('M')['x'].get('N')), float(pitch.get('spin').get('M')['y'].get('N')),
            float(pitch.get('spin').get('M')['z'].get('N'))]

    return {
        'pitch_id': pitch.get('pitch_id').get('S'),
        'positions': positions,
        'timestamps': timestamps,
        'error': errors,
        'spin': spin,
        'serial_number': int(pitch.get('serial_number').get('N')),
        'time': pitch.get('time').get('S'),
        'user_id': pitch.get('user_id').get('S')
    }
