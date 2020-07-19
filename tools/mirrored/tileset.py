from pixelgrid import *
from PIL import ImageTk, Image, ImageDraw
import json

TILES = [
    {
        "name": 'SPACE_EMPTY',
        "x": -2,
        "y": -2,
        "flip": False
    },
    {
        "name": "SPACE_PHOTON",
        "x": 0,
        "y": 0,
        "flip": False
    },
    {
        "name": "SPACE_ANTIPHOTON",
        "x": 0,
        "y": 8,
        "flip": False
    },
    {
        "name": "SPACE_RIGHT_LEFT_MIRROR",
        "x": 2,
        "y": 0,
        "flip": False
    },
    {
        "name": "SPACE_LEFT_RIGHT_MIRROR",
        "x": 2,
        "y": 0,
        "flip": True
    },
    {
        "name": "SPACE_RIGHT_LEFT_SOLMIR",
        "x": 2,
        "y": 2,
        "flip": False
    },
    {
        "name": "SPACE_LEFT_RIGHT_SOLMIR",
        "x": 2,
        "y": 2,
        "flip": True
    }
]

def getTileset():
    pixelgrid = PixelGrid([(0,0,0)])
    with open("../../images/sprites/lasers.terra", "r") as fileo:
        pixelgrid.load(json.load(fileo))
    
    tilesImgs = [
        pixelgrid.get16x16tile(tile["x"], tile["y"], tile["flip"])
        for tile in TILES
    ]
    tileset = [
        tile.resize((tile.width * 2, tile.height * 2), Image.NEAREST)
        for tile in tilesImgs
    ]
    tiles2x = Image.new("RGB",(
        tileset[0].width * len(tileset),
        tileset[0].height
    ))
    for idx, tile in enumerate(tileset):
        tiles2x.paste(tile, (idx * tile.width, 0))
    return (tiles2x, tileset, ImageTk.PhotoImage(tiles2x))