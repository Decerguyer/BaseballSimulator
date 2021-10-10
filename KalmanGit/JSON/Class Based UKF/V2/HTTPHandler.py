import requests


class HTTPHandler:

    # https://2342hbqxca.execute-api.us-east-1.amazonaws.com/dev
    # /pitch at end returns all
    # /pitch/ID at end returns singular
    def __init__(self, username: str):
        '''
        :param username: username of requested pitcher
        :type username: str
        '''
        self.username = username.replace(" ", "%20")

    def get_requested_pitches(self, url):
        requested_json = requests.get(url)
        print("Status Code for All Pitches Get Request:")
        print(requested_json.status_code)
        all_pitches = requested_json.json()
        all_pitches.sort(key=lambda x: x['time'])
        # print(all_pitches)
        return all_pitches

    def get_user_pitches(self):
        #url = "https://2342hbqxca.execute-api.us-east-1.amazonaws.com/dev/pitch?pitcher=" + self.username
        url = "https://2342hbqxca.execute-api.us-east-1.amazonaws.com/dev/pitch"
        # print(url)
        user_pitches = self.get_requested_pitches(url)
        return user_pitches

    def get_most_recent_pitch(self):
        all_pitches = self.get_user_pitches()
        return all_pitches[-1]

    def get_specific_pitch(self):
        pitch_id = 'f71c154e69dc42b4af88f69d806ed2fc'
        url = "https://2342hbqxca.execute-api.us-east-1.amazonaws.com/dev/pitch"
        final_url = url + '/' + pitch_id
        pitch = requests.get(final_url)
        return pitch.json()

    def post_to_JEY_backend(self, up_pitch: dict):
        url = 'https://pkafa2msue.execute-api.us-east-1.amazonaws.com/dev/pitch'
        #url = 'http://localhost:5000/pitch'
        print("Posting")
        x = requests.post(url, json=up_pitch)
        print(x.text)

    def get_pitches_timeframe(self, start_time: str, end_time: str, user_id: str):
        url = 'https://pkafa2msue.execute-api.us-east-1.amazonaws.com/dev/pitch/time?'
        url = url + 'start_time=' + start_time + '&end_time=' + end_time + "&user_id=" + user_id
        print("Getting this URL")
        print(url)
        x = requests.get(url)
        print(x.text)



