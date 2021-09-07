class Vector3D:
    def __init__(self, x: float, y: float, z: float):
        self.x = x
        self.y = y
        self.z = z

    def three_dim_to_dynamo_item(self):
        return {'M': {'x': {'N': str(self.x)}, 'y': {'N': str(self.y)}, 'z': {'N': str(self.z)}}}
