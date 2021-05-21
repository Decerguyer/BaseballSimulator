from flask import Flask, jsonify, request
from typing import List, Tuple, Dict
import os
import boto3
import math
import datetime
import uuid
import json


class Vector3D():
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z

    def to_dynamo_item(self):
        return { 'M': { 'x': { 'N': self.x }, 'y': { 'N': self.y }, 'z': { 'N': self.z } } }
class Pitch:
    def __init__(self, positions: List[Vector3D], timestamps: List[float], spin: Vector3D, pitcher_id: str, pitch_id: str, serial_number: int, error: List[Vector3D], time: str):
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
            dynamo_timestamps.append({ 'N': timestamp })
        return {
            'positions': { 'L': dynamo_positions },
            'timestamps': { 'L': dynamo_timestamps },
            'spin': self.spin.to_dynamo_item(),
            'error': { 'L': dynamo_error },
            'serial_number': { 'N': self.serial_number },
            'pitcher_id': { 'S': self.pitcher_id },
            'pitch_id': { 'S': self.pitch_id }            
        }
        

    def kalman_filter(self):
        print('filter the pitch')
        return self

    def simulate(self):
        print('simulate the rest of the pitch from the kalman filter')
        return self



app = Flask(__name__)

PICHES_TABLE = os.environ['PITCHES_TABLE']
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
    resp = client.get_item(
        TableName=PICHES_TABLE,
        Key={
            'pitch_id': {'S': pitch_id}
        }
    )
    item = resp.get('Item')
    if not item:
        return jsonify({'error': 'Pitch does not exist'}), 404

    # {
    #     'pitch_id': item.get('pitch_id').get('S'),
    #     'positions': item.get('positions').get('NS'),
    #     'spin': item.get('spin').get('N'),
    #     'timestamp': item.get('time').get('S'),
    #     'pitcher': item.get('pitcher').get('S')
    # }

    return jsonify(item), 200


@app.route("/pitch", methods=["GET"])
def get_pitch_history():
    resp = client.scan(
        TableName=PICHES_TABLE
    )
    result = []
    print('Response: ', resp)
    # for pitch in resp.get('Items'):
    #     # result.append({
    #     #     'pitch_id': pitch.get('pitch_id').get('S'),
    #     #     'positions': pitch.get('positions').get('NS'),
    #     #     'spin': pitch.get('spin').get('N'),
    #     #     'timestamp': pitch.get('time').get('S'),
    #     #     'pitcher': pitch.get('pitcher').get('S')
    #     # })

    return jsonify(resp.get('Items')), 200


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

        timestamps= request.json.get('timestamps')
        spin = Vector3D(*request.json.get('spin'))
        pitcher_id = request.json.get('pitcher_id')
        serial_number = request.json.get('serial_number')
        pitch = Pitch(positions, timestamps, spin, pitcher_id, pitch_id, serial_number, error_list, time)
        

        name = request.json.get('name')
        if not positions or not spin or not pitch_id:
            return jsonify({'error': 'Please provide positions, spin, and user'}), 400

        resp = client.put_item(
            TableName=PICHES_TABLE,
            Item=pitch.to_dynamo_item()
        )

        return jsonify({
            'success': 'recorded pitch with id: {}'.format(pitch_id)
        }), 201
    except(error):
        return jsonify({'error': error})

