from HTTPHandler import HTTPHandler

http = HTTPHandler("JEYSolutions")

'''
pulled_json = http.get_specific_pitch()
print(pulled_json)
del pulled_json['pitch_id']
del pulled_json['pitcher_id']
del pulled_json['time']

for k,b in pulled_json.items():
    print(k)
print(pulled_json)
http.post_to_JEY_backend(pulled_json)
'''

http.get_pitches_timeframe('2021-10-9', '2021-10-9', 'JEYSolutions')
