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
        all_pitches.sort(key=lambda x: x['timestamp'])
        # print(all_pitches)
        return all_pitches

    def get_user_pitches(self):
        url = "https://2342hbqxca.execute-api.us-east-1.amazonaws.com/dev/pitch?pitcher=" + self.username
        # print(url)
        user_pitches = self.get_requested_pitches(url)
        return user_pitches

    def get_most_recent_pitch(self):
        all_pitches = self.get_user_pitches()
        return all_pitches[-1]


