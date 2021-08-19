def convert_processed_pitch_to_response(pitch):
    positions = []
    for position in pitch.get('positions').get('L'):
        positions.append([float(position.get('M')['x'].get('N')), float(position.get('M')['y'].get('N')),
                          float(position.get('M')['z'].get('N'))])

    rts_positions = []
    for rts_position in pitch.get('rts_positions').get('L'):
        rts_positions.append([float(rts_position.get('M')['x'].get('N')), float(rts_position.get('M')['y'].get('N')),
                              float(rts_position.get('M')['z'].get('N'))])
    return {
        'pitch_id': pitch.get('pitch_id').get('S'),
        'positions': positions,
        'rts_positions': rts_positions,
        'serial_number': int(pitch.get('serial_number').get('N')),
        'time': pitch.get('time').get('S'),
        'user_id': pitch.get('user_id').get('S')
    }


def convert_unprocessed_pitch_to_response(pitch):
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


def get_user_from_serial(serial_number: int) -> str:
    return "dummy_user"


def get_mound_offset_from_serial(serial_number: int) -> str:
    return 10


def get_height_offset_from_serial(serial_number: int) -> str:
    return 10
