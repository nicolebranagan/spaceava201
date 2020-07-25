from pixelgrid import *
from PIL import ImageTk, Image, ImageDraw
import json

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

    return (tiles2x, tileset)

def getStarbase():
    (starbase2x, starbasetileset) = getTileset("../../images/tiles/stardrop.terra")

    tiles2x = Image.new("RGB",(
        starbase2x.width,
        starbase2x.height
    ))
    tiles2x.paste(starbase2x, (0, 0))
    return (tiles2x, starbasetileset, ImageTk.PhotoImage(tiles2x))

def getMirrorSys():
    (starbase2x, starbasetileset) = getTileset("../../images/tiles/mirrorsys.terra")

    tiles2x = Image.new("RGB",(
        starbase2x.width,
        starbase2x.height
    ))
    tiles2x.paste(starbase2x, (0, 0))
    return (tiles2x, starbasetileset, ImageTk.PhotoImage(tiles2x))

def getStarship():
    (starbase2x, starbasetileset) = getTileset("../../images/tiles/starship.terra")

    tiles2x = Image.new("RGB",(
        starbase2x.width,
        starbase2x.height
    ))
    tiles2x.paste(starbase2x, (0, 0))
    return (tiles2x, starbasetileset, ImageTk.PhotoImage(tiles2x))

def getSun():
    (starbase2x, starbasetileset) = getTileset("../../images/tiles/sunscape.terra")

    tiles2x = Image.new("RGB",(
        starbase2x.width,
        starbase2x.height
    ))
    tiles2x.paste(starbase2x, (0, 0))
    return (tiles2x, starbasetileset, ImageTk.PhotoImage(tiles2x))

def getTilesets():
    return [getStarbase(), getMirrorSys(), getStarship(), getSun()]
