from pixelgrid import *
from PIL import ImageTk, Image, ImageDraw
import json

TILESET_NAME = {
    "Starbase": 0,
    "Mirror System": 1,
    "Starship": 2,
    "Sunscape": 3,
    "Title Screen": 4,
    "Amalgamation HQ": 5
}

NAME_TILESET = {v: k for k, v in TILESET_NAME.items()}

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

    return (tiles2x, tileset, ImageTk.PhotoImage(tiles2x))

def getTilesets():
    return [
        getTileset("../../images/tiles/stardrop.terra"), 
        getTileset("../../images/tiles/mirrorsys.terra"), 
        getTileset("../../images/tiles/starship.terra"), 
        getTileset("../../images/tiles/sunscape.terra"), 
        getTileset("../../images/tiles/titles.terra"),
        getTileset("../../images/tiles/amalghq.terra"),
    ]
