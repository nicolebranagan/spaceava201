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
    "PLAY_LOADED_SFX": 9,
    "LOAD_NEXT_SEGMENT": 10
}

SPRITES = {
    "AVA": 1,
    "CINDY": 2,
    "NELEHU": 3,
    "BOB": 4,
    "GOON": 5,
    "BUNNY": 6,
    "BAGBUNNY": 7,
    "MOSSBAU": 8,
    "ANTONIA": 9
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
    "SUNSCAPE_1": 8,
    "NEPTUNE_OUTSIDE": 9,
    "STARSHIP_NEPTUNE": 10,
    "NEPTUNE_SURFACE": 11
}

BACKGROUND_GROUPING = {
    "STARBASE": 0,
    "VOID/STARSHIP": 1,
    "SUNSCAPE": 2,
    "NEPTUNE": 3
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
    "SUNSCAPE_1": BACKGROUND_GROUPING["SUNSCAPE"],
    "NEPTUNE_OUTSIDE": BACKGROUND_GROUPING["NEPTUNE"],
    "STARSHIP_NEPTUNE": BACKGROUND_GROUPING["VOID/STARSHIP"],
    "NEPTUNE_SURFACE": BACKGROUND_GROUPING["NEPTUNE"]
}

TRACKS = {
    "Ballad of St. Janet": 0,
    "Evan S. Sense": 1,
    "Chime 2020": 2,
    "Lacked Even a Daisy": 3,
    "Seventeen Crowns": 4
}

SFXES = {
    "Kaboom": 0,
    "Evil laugh": 1
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
    cast = []

    current_bg_grouping = -1

    last_slot = 0
    last_sprite = [0 for _ in range(0, 10)]
    last_face = [0 for _ in range(0, 10)]
    last_x = [0 for _ in range(0, 10)]

    last_slot_for_face = {}

    commands = []

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

            commands.append(bytes(
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
                commands.append(
                    bytes([COMMANDS["SHOW_TEXT"], chirp]) + 
                    newtext.encode('ascii') + 
                    bytes([0])
                )
        elif (command["command"] == "SHOW_CENTERED_TEXT"):
            newtext = parse_and_center_text(command["text"])
            chirp = int(command["chirp"]) if "chirp" in command else 0
            commands.append(
                bytes([COMMANDS["SHOW_TEXT"], chirp]) + 
                newtext.encode('ascii') + 
                bytes([0])
            )
        elif (command["command"] == "SHOW_BACKGROUND"):
            newbg = command["background"]
            newgroup = BACKGROUNDS_TO_BACKGROUND_GROUPING[newbg]
            commands.append(
                bytes(
                    [COMMANDS["SHOW_BACKGROUND"],
                    BACKGROUNDS[newbg],
                    0 if newgroup == current_bg_grouping else 1]
                )
            )
            current_bg_grouping = newgroup
        elif (command["command"] == "PLAY_MUSIC"):
            commands.append(
                bytes(
                    [COMMANDS["PLAY_MUSIC"],
                    TRACKS[command["track"]]]
                )
            )
        elif (command["command"] == "STOP_MUSIC"):
            commands.append(bytes([COMMANDS["STOP_MUSIC"]]))
        elif (command["command"] == "SHOW_FRAME"):
            commands.append(bytes([COMMANDS["SHOW_FRAME"], command["frame"]]))
        elif (command["command"] == "DELETE_SPRITE"):
            commands.append(bytes([COMMANDS["DELETE_SPRITE"], command["slot"]]))
        elif (command["command"] == "LOAD_SFX"):
            commands.append(
                bytes(
                    [COMMANDS["LOAD_SFX"],
                    SFXES[command["sfx"]]]
                )
            )
        elif (command["command"] == "PLAY_LOADED_SFX"):
            commands.append(bytes([COMMANDS["PLAY_LOADED_SFX"]]))
    
    last_song = -1
    blocks = []
    current_block = bytes(cast) + bytes([255])
    for cmd in commands:
        if (len(current_block + cmd) > (2048 - 3)): 
            print("Expanding block")
            current_block = current_block + (
                bytes([COMMANDS["LOAD_NEXT_SEGMENT"]])
            )
            blocks.append(current_block)
            if last_song > -1:
                current_block = bytes([
                    COMMANDS["PLAY_MUSIC"],
                    last_song
                ]) + cmd
            else:
                current_block = cmd
        else:
            if (cmd[0] == COMMANDS["PLAY_MUSIC"]):
                last_song = cmd[1]
            elif (cmd[0] == COMMANDS["STOP_MUSIC"]):
                last_song = -1
            current_block = current_block + cmd
    blocks.append(current_block + bytes([255]))

    return blocks

if (len(sys.argv) < 3):
    print("usage: story.py <input> <output>")
    sys.exit()

inputfile = sys.argv[1]
outputfile = sys.argv[2]

with open(inputfile, "r") as fileo:
    input = json.load(fileo)

output = b''
parsed_scripts = []
for script in input:
    parsed_scripts = parsed_scripts + parse_single_script(script)
for parsed_script in parsed_scripts:
    size = len(parsed_script)
    sectorcount = math.ceil(size / 2048)
    if (sectorcount > 1):
        raise ValueError("Too long!")
    paddeddata = parsed_script + bytes([0 for _ in range(0, (sectorcount * 2048) - size)])
    output = output + paddeddata

with open(outputfile, "wb") as outputo:
    outputo.write(output)
