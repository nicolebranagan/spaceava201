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

def getHarsh():
    (betel2x, beteltileset) = getTileset("../../images/tiles/harsh.terra")
    (betelrot2x, betelrottileset) = getTileset("../../images/tiles/harshout.terra")

    tiles2x = Image.new("RGB",(
        betel2x.width + betelrot2x.width, 
        betel2x.height, 
    ))
    tiles2x.paste(betel2x, (0, 0))
    tiles2x.paste(betelrot2x, (betel2x.width, 0))
    return (tiles2x, beteltileset + betelrottileset, ImageTk.PhotoImage(tiles2x))

def getTilesets():
    return [getHarsh() ]
