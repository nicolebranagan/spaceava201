# turboize.py
#
# usage: turboize.py [sprite|tile] <input> <output> <pal_output>

from pixelgrid import *
import sys
import json

def palettize(inputgrid):
    def to_binary(input):
        return bin(input)[2:].zfill(3) # strip 0b and make 3 digits
    output = []
    palette = inputgrid.palette[0:16]
    for color in palette:
        red = to_binary(color[0] // 32)
        green = to_binary(color[1] // 32)
        blue = to_binary(color[2] // 32)
        output.append(int(f'{green[1:]}{red}{blue}', 2))
        output.append(int(f'0000000{green[0]}', 2))
    return output

def index_to_binary(input):
    return bin(input)[2:].zfill(4) # strip 0b and make 4 digits

def turboize_tile(inputgrid, x_start, y_start):
    bitplane1 = []
    bitplane2 = []
    bitplane3 = []
    bitplane4 = []
    
    for y in range(0, 8):
        rowplane1 = []
        rowplane2 = []
        rowplane3 = []
        rowplane4 = []
        for x in range(0, 8):
            true_x = x + x_start
            true_y = y + y_start
            val = index_to_binary(inputgrid.get(true_x, true_y))
            rowplane1.append(val[3])
            rowplane2.append(val[2])
            rowplane3.append(val[1])
            rowplane4.append(val[0])
        bitplane1.append(int(''.join(rowplane1), 2))
        bitplane2.append(int(''.join(rowplane2), 2))
        bitplane3.append(int(''.join(rowplane3), 2))
        bitplane4.append(int(''.join(rowplane4), 2))
    
    output = []
    for i in range(0, 8):
        output.append(bitplane1[i])
        output.append(bitplane2[i])
    for i in range(0, 8):
        output.append(bitplane3[i])
        output.append(bitplane4[i])
    return output

def turboize_tiles(inputgrid):
    max_ = inputgrid.getmaxtuple()
    rows = math.ceil((1+max_[1])/2)
    output = []
    for ygroup in range(0, rows):
        for tile in range(0, inputgrid.width // 2):
            true_x = tile * 16
            true_y = ygroup * 16
            output.extend(turboize_tile(inputgrid, true_x, true_y))
            output.extend(turboize_tile(inputgrid, true_x + 8, true_y))
            output.extend(turboize_tile(inputgrid, true_x, true_y + 8))
            output.extend(turboize_tile(inputgrid, true_x + 8, true_y + 8))
    return output

def turboize_sprite(inputgrid):
    output = []
    for ygroup in range(0, inputgrid.height // 2):
        bitplane1 = []
        bitplane2 = []
        bitplane3 = []
        bitplane4 = []
        for y in range(0, 16):
            rowplane1 = []
            rowplane2 = []
            rowplane3 = []
            rowplane4 = []
            for x in range(0, 16):
                true_x = x
                true_y = y + 16 * ygroup
                val = index_to_binary(inputgrid.get(true_x, true_y))
                rowplane1.append(val[3])
                rowplane2.append(val[2])
                rowplane3.append(val[1])
                rowplane4.append(val[0])
            bitplane1.append(int(''.join(rowplane1[8:]), 2))
            bitplane1.append(int(''.join(rowplane1[:8]), 2))

            bitplane2.append(int(''.join(rowplane2[8:]), 2))
            bitplane2.append(int(''.join(rowplane2[:8]), 2))

            bitplane3.append(int(''.join(rowplane3[8:]), 2))
            bitplane3.append(int(''.join(rowplane3[:8]), 2))

            bitplane4.append(int(''.join(rowplane4[8:]), 2))
            bitplane4.append(int(''.join(rowplane4[:8]), 2))
        output.extend(bitplane1 + bitplane2 + bitplane3 + bitplane4)
    return output

if (len(sys.argv) < 5):
    print("usage: turboize.py [sprite|tile] <input> <output> <palette>")
    sys.exit()

mode = sys.argv[1]
inputfile = sys.argv[2]
outputfile = sys.argv[3]
palettefile = sys.argv[4]

pixelgrid = PixelGrid([(0,0,0)])

with open(inputfile, "r") as fileo:
    pixelgrid.load(json.load(fileo))

if mode == "sprite":
    bytelist = turboize_sprite(pixelgrid)
elif mode == "tile":
    bytelist = turboize_tiles(pixelgrid)
else:
    print("Unsupported type")
    sys.exit(2)

with open(outputfile, "wb") as fileo:
    fileo.write(bytes(bytelist))

palette = palettize(pixelgrid)
if mode == "sprite":
    palette[0] = 0
with open(palettefile, "wb") as fileo:
    fileo.write(bytes(palette))