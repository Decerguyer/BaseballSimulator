from flask import Flask, jsonify, request
from typing import List, Tuple, Dict
import os
import boto3
from boto3.dynamodb.conditions import Key
import math
import datetime
import uuid
import json
from flask_cors import CORS

from utils import convert_processed_pitch_to_response, convert_config_to_response
from pitch import Pitch
from vector3D import Vector3D
from sys_config import SysConfig

app = Flask(__name__)
CORS(app)

UNPROCESSED_PITCHES_TABLE = os.environ.get('UNPROCESSED_PITCHES_TABLE')
PROCESSED_PITCHES_TABLE = os.environ.get('PROCESSED_PITCHES_TABLE')
CONFIGURATION_TABLE = os.environ.get('CONFIGURATION_TABLE')
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

        return jsonify(convert_processed_pitch_to_response(pitch)), 200
    except Exception as e:
        return jsonify({'error': str(e)}), 400


@app.route("/pitch/time")
def get_pitch():
    try:
        start_time = str(request.args['start_time'])
        end_time = str(request.args['end_time'])
        resp = client.query(
            TableName=PROCESSED_PITCHES_TABLE,
            KeyConditionExpression='user_id = :user AND #t BETWEEN :start AND :end',
            ExpressionAttributeValues={
                ':user': {'S': 'JEYSolutions'},
                ':start': {'S': start_time},
                ':end': {'S': end_time}
            },
            ExpressionAttributeNames={"#t": "time"}
        )
        pitches = resp.get('Items')
        if not pitches:
            return jsonify({'error': 'Pitches do not exist'}), 404
        result = []
        for pitch in pitches:
            result.append(convert_processed_pitch_to_response(pitch))
        return jsonify(result), 200
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
            result.append(convert_processed_pitch_to_response(pitch))
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
        config_dict = get_config_from_sn(up_dict['serial_number'])

        if(config_dict):
            up_dict['user_id'] = config_dict['user_id']
            up_dict['mound_offset'] = config_dict['mound_offset']
            up_dict['height_offset'] = config_dict['height_offset']
        else:
            return jsonify({'error': 'This System has not been configured'}), 400

        # RELEVANT TO KF: positions, timestamps, spin, error_list
        pitch = Pitch(up_dict)
        pitch.kalman_filter()

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


@app.route("/config", methods=["POST"])
def record_config():
    try:
        config_dict = {}
        config_dict['serial_number'] = request.json.get('serial_number')
        config_dict['user_id'] = request.json.get('user_id')
        config_dict['mound_offset'] = request.json.get('mound_offset')
        config_dict['height_offset'] = request.json.get('height_offset')

        if not config_dict['serial_number'] or not config_dict['user_id'] or not config_dict['mound_offset'] \
                or not config_dict['height_offset']:
            return jsonify({'error': 'Please provide valid positions, spin and error'}), 400

        config = SysConfig(config_dict)

        # post to processed table second
        p_resp = client.put_item(
            TableName=CONFIGURATION_TABLE,
            Item=config.to_processed_dynamo_item()
        )

        return jsonify({
            'success': 'recorded config for SN: {}'.format(config_dict['serial_number'])
        }), 201
    except Exception as e:
        return jsonify({'error': str(e)}), 400


def get_config_from_sn(serial_number: int):
    try:
        resp = client.get_item(
            TableName=CONFIGURATION_TABLE,
            Key={
                'serial_number': {'N': str(serial_number)}
            }
        )
        config = resp.get('Item')
        if not config:
            return False
        return convert_config_to_response(config)
    except Exception as e:
        print(e)
        return str(e)
