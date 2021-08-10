from flask import Flask, jsonify, request
from typing import List, Tuple, Dict
import os
import boto3
import math
import datetime
import uuid
import json
from flask_cors import CORS


class Vector3D():
    def __init__(self, x: float, y: float, z: float):
        self.x = x
        self.y = y
        self.z = z

    def to_dynamo_item(self):
        return {'M': {'x': {'N': str(self.x)}, 'y': {'N': str(self.y)}, 'z': {'N': str(self.z)}}}


class Pitch:
    def __init__(self, positions: List[Vector3D], timestamps: List[float], spin: Vector3D, pitcher_id: str,
                 pitch_id: str, serial_number: int, error: List[Vector3D], time: str):
        self.positions = positions
        self.timestamps = timestamps
        self.spin = spin
        self.pitcher_id = pitcher_id
        self.serial_number = serial_number
        self.pitch_id = pitch_id
        self.error = error
        self.time = time

    def to_dynamo_item(self):
        dynamo_positions = []
        dynamo_timestamps = []
        dynamo_error = []
        for position in self.positions:
            dynamo_positions.append(position.to_dynamo_item())
        for err in self.error:
            dynamo_error.append(err.to_dynamo_item())
        for timestamp in self.timestamps:
            dynamo_timestamps.append({'N': str(timestamp)})
        return {
            'positions': {'L': dynamo_positions},
            'timestamps': {'L': dynamo_timestamps},
            'spin': self.spin.to_dynamo_item(),
            'error': {'L': dynamo_error},
            'serial_number': {'N': str(self.serial_number)},
            'user_id': {'S': str(self.user_id)},
            'pitch_id': {'S': str(self.pitch_id)},
            'time': {'S': str(self.time)}
        }

    def kalman_filter(self):
        print('filter the pitch')
        return self

    def simulate(self):
        print('simulate the rest of the pitch from the kalman filter')
        return self


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
            TableName=PITCHES_TABLE,
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


def convert_pitch_to_response(pitch):
    positions = []
    timestamps = []
    errors = []
    spins = []
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


@app.route("/pitch", methods=["GET"])
def get_pitch_history():
    try:
        resp = client.scan(
            TableName=PITCHES_TABLE
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
        pitch_id = uuid.uuid4().hex
        print(pitch_id)
        time = str(datetime.datetime.now())
        positions = []
        for position in request.json.get('positions'):
            positions.append(Vector3D(*position))
        error_list = []
        for error in request.json.get('error'):
            error_list.append(Vector3D(*error))

        timestamps = request.json.get('timestamps')
        spin = Vector3D(*request.json.get('spin'))
        user_id = request.json.get('user_id')
        serial_number = request.json.get('serial_number')
        pitch = Pitch(positions, timestamps, spin, user_id, pitch_id, serial_number, error_list, time)

        #RELEVANT TO KF: positions, timestamps, spin, error_list,

        name = request.json.get('name')
        if not positions or not spin or not pitch_id:
            return jsonify({'error': 'Please provide positions, spin, and user'}), 400

        resp = client.put_item(
            TableName=PITCHES_TABLE,
            Item=pitch.to_dynamo_item()
        )

        return jsonify({
            'success': 'recorded pitch with id: {}'.format(pitch_id)
        }), 201
    except Exception as e:
        return jsonify({'error': str(e)}), 400
