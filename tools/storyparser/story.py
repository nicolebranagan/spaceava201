#
# Parses story files
#

import sys
import json
import textwrap
import math

COMMANDS = {
    "SHOW_SPRITE": 1,
    "SHOW_TEXT": 2,
    "SHOW_BACKGROUND": 3
}

SPRITES = {
    "AVA": 1,
    "CINDY": 2,
    "NELEHU": 3,
    "BOB": 4
}

BACKGROUNDS = {
    "STARBASE": 0,
    "STARBASE_EXTERIOR": 1,
    "TRAINING_HALL": 2
}

SCREEN_WIDTH = 36
LINE_COUNT = 4

def parse_and_center_text(text):
    initlines = text.split('\n')
    wrappedlines = []
    for line in initlines:
        lines = textwrap.wrap(line, SCREEN_WIDTH)
        for wrappedline in lines:
            length = len(wrappedline)
            dellength = (SCREEN_WIDTH - length) // 2
            wrappedlines.append(wrappedline.rjust(dellength + length))
    return '\n'.join(wrappedlines)

def parse_text(text):
    lines = textwrap.wrap(text, SCREEN_WIDTH)
    return ['\n'.join(lines[(x*4):((x+1)*4)]) for x in range(0, math.ceil(len(lines) / 4))]

def parse_single_script(script):
    output = b''
    cast = []

    last_slot = 0
    last_sprite = [0 for _ in range(0, 10)]
    last_face = [0 for _ in range(0, 10)]
    last_x = [0 for _ in range(0, 10)]

    for command in script:
        if (command["command"] == "SHOW_SPRITE"):
            if ("sprite" in command and SPRITES[command["sprite"]] not in cast):
                cast.append(SPRITES[command["sprite"]])
            
            slot = last_slot
            if "slot" in command:
                slot = command["slot"]
                last_slot = command["slot"]
            if "sprite" in command:
                last_sprite[slot] = command["sprite"]
            if "face" in command:
                last_face[slot] = command["face"]
            if "x" in command:
                last_x[slot] = command["x"]

            output = (
                output + bytes(
                    [COMMANDS["SHOW_SPRITE"],
                    slot,
                    cast.index(SPRITES[last_sprite[slot]]),
                    last_face[slot],
                    last_x[slot]]
                )
            )
        elif (command["command"] == "SHOW_TEXT"):
            parsedsets = parse_text(command["text"])
            for newtext in parsedsets:
                output = (
                    output + 
                    bytes([COMMANDS["SHOW_TEXT"]]) + 
                    newtext.encode('ascii') + 
                    bytes([0])
                )
        elif (command["command"] == "SHOW_CENTERED_TEXT"):
            newtext = parse_and_center_text(command["text"])
            output = (
                output + 
                bytes([COMMANDS["SHOW_TEXT"]]) + 
                newtext.encode('ascii') + 
                bytes([0])
            )
        elif (command["command"] == "SHOW_BACKGROUND"):
            output = (
                output + bytes(
                    [COMMANDS["SHOW_BACKGROUND"],
                    BACKGROUNDS[command["background"]]]
                )
            )
    return bytes(cast) + bytes([255]) + output + bytes([255])

if (len(sys.argv) < 3):
    print("usage: story.py <input> <output>")
    sys.exit()

inputfile = sys.argv[1]
outputfile = sys.argv[2]

with open(inputfile, "r") as fileo:
    input = json.load(fileo)

output = b''
for script in input:
    parsed_script = parse_single_script(script)
    size = len(parsed_script)
    sectorcount = math.ceil(size / 2048)
    if (sectorcount > 1):
        raise ValueError("Too long!")
    paddeddata = parsed_script + bytes([0 for _ in range(0, (sectorcount * 2048) - size)])
    output = output + paddeddata

with open(outputfile, "wb") as outputo:
    outputo.write(output)
