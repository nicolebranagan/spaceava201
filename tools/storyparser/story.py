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
    "SHOW_BACKGROUND": 3,
    "PLAY_MUSIC": 4,
    "STOP_MUSIC": 5,
    "SHOW_FRAME": 6,
    "DELETE_SPRITE": 7,
    "LOAD_SFX": 8,
    "PLAY_LOADED_SFX": 9
}

SPRITES = {
    "AVA": 1,
    "CINDY": 2,
    "NELEHU": 3,
    "BOB": 4,
    "GOON": 5
}

BACKGROUNDS = {
    "STARBASE": 0,
    "STARBASE_EXTERIOR": 1,
    "TRAINING_HALL": 2,
    "VOID": 3,
    "STARSHIP_FEYNMAN": 4,
    "STARSHIP_REDALERT": 5,
    "STARSHIP_BETELGEUSE": 6,
    "STARSHIP_BETELGEUSE_ALERT": 7,
    "SUNSCAPE_1": 8
}

BACKGROUND_GROUPING = {
    "STARBASE": 0,
    "VOID/STARSHIP": 1,
    "SUNSCAPE": 2
}

BACKGROUNDS_TO_BACKGROUND_GROUPING = {
    "STARBASE": BACKGROUND_GROUPING["STARBASE"],
    "STARBASE_EXTERIOR": BACKGROUND_GROUPING["STARBASE"],
    "TRAINING_HALL": BACKGROUND_GROUPING["STARBASE"],
    "VOID": BACKGROUND_GROUPING["VOID/STARSHIP"],
    "STARSHIP_FEYNMAN": BACKGROUND_GROUPING["VOID/STARSHIP"],
    "STARSHIP_REDALERT": BACKGROUND_GROUPING["VOID/STARSHIP"],
    "STARSHIP_BETELGEUSE": BACKGROUND_GROUPING["VOID/STARSHIP"],
    "STARSHIP_BETELGEUSE_ALERT": BACKGROUND_GROUPING["VOID/STARSHIP"],
    "SUNSCAPE_1": BACKGROUND_GROUPING["SUNSCAPE"]
}

TRACKS = {
    "Ballad of St. Janet": 0,
    "Evan S. Sense": 1,
    "Chime 2020": 2,
    "Lacked Even a Daisy": 3
}

SFXES = {
    "Kaboom": 0
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

    current_bg_grouping = -1

    last_slot = 0
    last_sprite = [0 for _ in range(0, 10)]
    last_face = [0 for _ in range(0, 10)]
    last_x = [0 for _ in range(0, 10)]

    last_slot_for_face = {}

    for command in script:
        if (command["command"] == "SHOW_SPRITE"):
            if ("sprite" in command and SPRITES[command["sprite"]] not in cast):
                cast.append(SPRITES[command["sprite"]])
            
            slot = last_slot
            if "slot" in command:
                slot = command["slot"]
                last_slot = command["slot"]
            elif "sprite" in command:
                slot = last_slot_for_face[command["sprite"]]

            if "sprite" in command and "slot" in command:
                last_sprite[slot] = command["sprite"]
                last_slot_for_face[command["sprite"]] = slot
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
            chirp = int(command["chirp"]) if "chirp" in command else 0
            for newtext in parsedsets:
                output = (
                    output + 
                    bytes([COMMANDS["SHOW_TEXT"], chirp]) + 
                    newtext.encode('ascii') + 
                    bytes([0])
                )
        elif (command["command"] == "SHOW_CENTERED_TEXT"):
            newtext = parse_and_center_text(command["text"])
            chirp = int(command["chirp"]) if "chirp" in command else 0
            output = (
                output + 
                bytes([COMMANDS["SHOW_TEXT"], chirp]) + 
                newtext.encode('ascii') + 
                bytes([0])
            )
        elif (command["command"] == "SHOW_BACKGROUND"):
            newbg = command["background"]
            newgroup = BACKGROUNDS_TO_BACKGROUND_GROUPING[newbg]
            output = (
                output + bytes(
                    [COMMANDS["SHOW_BACKGROUND"],
                    BACKGROUNDS[newbg],
                    0 if newgroup == current_bg_grouping else 1]
                )
            )
            current_bg_grouping = newgroup
        elif (command["command"] == "PLAY_MUSIC"):
            output = (
                output + bytes(
                    [COMMANDS["PLAY_MUSIC"],
                    TRACKS[command["track"]]]
                )
            )
        elif (command["command"] == "STOP_MUSIC"):
            output = output + bytes([COMMANDS["STOP_MUSIC"]])
        elif (command["command"] == "SHOW_FRAME"):
            output = output + bytes([COMMANDS["SHOW_FRAME"], command["frame"]])
        elif (command["command"] == "DELETE_SPRITE"):
            output = output + bytes([COMMANDS["DELETE_SPRITE"], command["slot"]])
        elif (command["command"] == "LOAD_SFX"):
            output = (
                output + bytes(
                    [COMMANDS["LOAD_SFX"],
                    SFXES[command["sfx"]]]
                )
            )
        elif (command["command"] == "PLAY_LOADED_SFX"):
            output = output + bytes([COMMANDS["PLAY_LOADED_SFX"]])

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
