
import json

def truncate(n, decimals=0):
    multiplier = 10 ** decimals
    return int(n * multiplier) / multiplier


with open("/home/juanmartos/Downloads/OutputFile.json") as f:
    data =  json.load(f)[::20]


out = []
for coords in data:
    coords[0] = coords[0] +0.5
    coords[2] = coords[2] -1.5
    tmp = [truncate(elem, 2) for elem in coords]
    out.append(tmp)
print(out)

