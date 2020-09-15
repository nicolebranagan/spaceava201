from pixelgrid import *
from PIL import ImageTk, Image, ImageDraw
import json
import math

TILESET_NAME = {
    "Starbase": 0,
    "Mirror System": 1,
    "Starship": 2,
    "Sunscape": 3,
    "Title Screen": 4,
    "Amalgamation HQ": 5,
    "Intro text": 6,
    "Shut up and Calculate": 7,
    "Neptile": 8,
    "Harsh Art": 9,
    "Harsh Moderne": 10,
    "Star Rotate": 11
}

NAME_TILESET = {v: k for k, v in TILESET_NAME.items()}

MAX_TILESET_WIDTH = 24

def getTilePalette(tileset):
    height = math.ceil(len(tileset) // 16) * 32
    width = MAX_TILESET_WIDTH * 32

    palette = Image.new("RGB", (width, height))
    for idx, tile in enumerate(tileset):
        x = idx % MAX_TILESET_WIDTH
        y = idx // MAX_TILESET_WIDTH
        palette.paste(tile, (x*32, y*32))
    return ImageTk.PhotoImage(palette)

def getTileset(url):
    pixelgrid = PixelGrid([(0,0,0)])
    with open(url, "r") as fileo:
        pixelgrid.load(json.load(fileo))
    tiles = pixelgrid.getStrip(2)
    tiles2x = tiles.resize(
            (tiles.width * 2, tiles.height * 2),
            Image.NEAREST)
    tileset = [tiles2x.crop((x*32,0,(x+1)*32,32)) for x in
                range(0, int(tiles.width // 16))]

    return (tiles2x, tileset, getTilePalette(tileset))

def getTilesets():
    return [
        getTileset("../../images/tiles/stardrop.terra"), 
        getTileset("../../images/tiles/mirrorsys.terra"), 
        getTileset("../../images/tiles/starship.terra"), 
        getTileset("../../images/tiles/sunscape.terra"), 
        getTileset("../../images/tiles/titles.terra"),
        getTileset("../../images/tiles/amalghq.terra"),
        getTileset("../../images/tiles/introtxt.terra"),
        getTileset("../../images/tiles/shutcalc.terra"),
        getTileset("../../images/tiles/nepdrop.terra"),
        getTileset("../../images/tiles/harshart.terra"),
        getTileset("../../images/tiles/harshbg.terra"),
        getTileset("../../images/tiles/starrot.terra")
    ]
