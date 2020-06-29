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
    tileset = [tiles.crop((x*16,0,(x+1)*16,16)) for x in
                range(0, int(tiles.width // 16))]

    return (tiles2x, tileset)

def getStarbase():
    (starbase2x, starbasetileset) = getTileset("../../images/tiles/starbase.terra")
    (starrot2x, starrottileset) = getTileset("../../images/tiles/starrot.terra")

    tiles2x = Image.new("RGB",(
        starbase2x.width + starrot2x.width, 
        starbase2x.height, 
    ))
    tiles2x.paste(starbase2x, (0, 0))
    tiles2x.paste(starrot2x, (starbase2x.width, 0))
    return (tiles2x, starbasetileset + starrottileset, ImageTk.PhotoImage(tiles2x))
