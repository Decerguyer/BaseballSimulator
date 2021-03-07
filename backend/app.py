# app.py

import os

import boto3
import uuid
import datetime

from flask import Flask, jsonify, request
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

    return jsonify({
        'pitch_id': item.get('pitch_id').get('S'),
        'positions': item.get('positions').get('NS'),
        'spin': item.get('spin').get('N'),
        'timestamp': item.get('time').get('S'),
        'pitcher': item.get('pitcher').get('S')
    })

@app.route("/pitch", methods=["GET"])
def get_pitch_history():
    resp = client.scan(
        TableName=PICHES_TABLE
    )
    result = []
    print('Response: ', resp)
    for pitch in resp.get('Items'):
        result.append({
            'pitch_id': pitch.get('pitch_id').get('S'),
            'positions': pitch.get('positions').get('NS'),
            'spin': pitch.get('spin').get('N'),
            'timestamp': pitch.get('time').get('S'),
            'pitcher': pitch.get('pitcher').get('S')
        })

    return jsonify(result), 200


@app.route("/pitch", methods=["POST"])
def record_pitch():
    pitch_id = uuid.uuid4()
    positions = request.json.get('positions')
    spin = request.json.get('spin')
    pitcher = request.json.get('pitcher')
    time = datetime.datetime.now()

    name = request.json.get('name')
    if not positions or not spin or not pitcher:
        return jsonify({'error': 'Please provide positions, spin, and user'}), 400

    resp = client.put_item(
        TableName=PICHES_TABLE,
        Item={
                'pitch_id': {'S': str(pitch_id)},
                'positions': {'NS': positions},
                'spin': {'N': spin},
                'time': {'S': str(time)},
                'pitcher': {'S': pitcher}
                }
    )

    return jsonify({
        'success': 'recorded pitch with id: {}'.format(pitch_id)
    }), 201