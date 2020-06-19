# turboize.py
#
# usage: turboize.py <input> <output> <pal_output>

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

def turboize(inputgrid):
    def to_binary(input):
        return bin(input)[2:].zfill(4) # strip 0b and make 4 digits

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
                val = to_binary(inputgrid.get(true_x, true_y))
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

if (len(sys.argv) < 4):
    print("usage: turboize.py <input> <output> <palette>")
    sys.exit()

inputfile = sys.argv[1]
outputfile = sys.argv[2]
palettefile = sys.argv[3]

pixelgrid = PixelGrid([(0,0,0)])

with open(inputfile, "r") as fileo:
    pixelgrid.load(json.load(fileo))

bytelist = turboize(pixelgrid)
with open(outputfile, "wb") as fileo:
    fileo.write(bytes(bytelist))

palette = palettize(pixelgrid)
palette[0] = 0
with open(palettefile, "wb") as fileo:
    fileo.write(bytes(palette))