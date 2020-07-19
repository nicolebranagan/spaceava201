# From terraformer
#
# Canonical copy at 
# https://github.com/nicolebranagan/terraformer/blob/master/terralib/pixelgrid.py

import math
import copy
import tkinter as tk
from PIL import Image, ImageTk, ImageDraw
import pixelgrid

class PixelGrid:
    def __init__(self, palette):
        # Declare an empty pixelgrid
        self.width = 32
        self.height = 32

        self._tiles = {}

        self._pages = []
        self._pages.append(self._tiles)
        self.palette = palette

    @property
    def pages(self):
        return len(self._pages)

    def get(self, x, y, tileset=None):
        if tileset is None:
            tileset = self._tiles

        tilex = x // 8
        tiley = y // 8
        relx = x - tilex*8
        rely = y - tiley*8
        
        if ( (tilex, tiley) in tileset):
            return tileset[(tilex,tiley)].get(relx, rely)
        else:
            return 0

    def getColor(self, x, y, tileset=None):
        if tileset is None:
            tileset = self._tiles
        return self.palette[self.get(x,y,tileset)]

    def set(self, x, y, val):
        if x < 0 or x >= self.width*8 or y < 0 or y >= self.height*8:
            return

        tilex = x // 8
        tiley = y // 8
        relx = x - tilex*8
        rely = y - tiley*8

        if ( (tilex, tiley) not in self._tiles):
            self._tiles[(tilex,tiley)] = PixelTile()
        self._tiles[(tilex,tiley)].set(relx, rely, val)

    def clearTile(self, x, y): 
        self._tiles.pop((x,y),None)

    def bounds(self, x, y):
        return (x >= 0 and x < 8*self.width and 
                y >= 0 and y < 8*self.height)

    def mergeSubset(self, subset, x, y):
        newtiles = copy.deepcopy(subset._tiles)
        for i in range(0, subset.width):
            for j in range(0, subset.height):
                if i + x <= self.width and j + y <= self.height:
                    if (i,j) in newtiles:
                        self._tiles[(i+x, j+y)] = newtiles[(i,j)]
                    else:
                        if (i+x, j+y) in self._tiles:
                            del(self._tiles[(i+x, j+y)])
    
    def flipColors(self, val1, val2, s):
        for i in range(s[0], s[2]):
            for j in range(s[1], s[3]):
                if (i,j) in self._tiles:
                    self._tiles[(i,j)].flip(val1, val2)
                elif val1 == 0:
                    self._tiles[(i, j)] = PixelTile(val2)

    def linearshift(self, a, b, c, d, e, f):
        oldtiles = copy.deepcopy(self._tiles)
        for x in range(0, 8*self.width):
            for y in range(0, 8*self.height):
                i = (a*x+b*y+c*(self.width*8-1))
                j = (d*x+e*y+f*(self.height*8-1))
                if (self.get(x, y, oldtiles) != self.get(i,j, oldtiles)):
                    self.set(i,j,self.get(x,y,oldtiles))

    def shift(self, dx, dy):
        oldtiles = copy.deepcopy(self._tiles)
        
        for i in range(0, 8*self.width):
            for j in range(0, 8*self.height):
                x = (i - dx) % (8*self.width)
                y = (j - dy) % (8*self.height)
                 
                if (self.get(x, y, oldtiles) != self.get(i,j, oldtiles)):
                    self.set(i,j,self.get(x,y,oldtiles))

    def getTkImage(self, zoom, block=True):
        photo = tk.PhotoImage(width=8*self.width*zoom, 
                              height=8*self.height*zoom)
        if block:
            photo.put("#%02x%02x%02x" % self.palette[0], 
                      to=(0,0,8*self.width*zoom,8*self.height*zoom))
        for i in range(0, 8*self.width):
            for j in range(0, 8*self.height):
                if self.get(i,j) != 0:
                    photo.put(
                        "#%02x%02x%02x" % self.getColor(i,j), 
                        to=(i*zoom, j*zoom,i*zoom+(zoom), j*zoom+(zoom)))
        return photo

    def drawTkSubset(self, photo, zoom, x, y, rx, ry, nx, ny, block=True, p=-1):
        if block:
            photo.put("#%02x%02x%02x" % self.palette[0], 
                      to=(nx,ny,nx+(8*rx*zoom),ny+(8*ry*zoom)))
        if p == -1:
            tileset = self._tiles
        else:
            tileset = self._pages[p]    

        for i in range(0, 8*rx):
            for j in range(0, 8*ry):
                if self.get(i + x*8,j + y*8, tileset) != 0:
                    photo.put(
                        "#%02x%02x%02x" % self.getColor(i + x*8,j + y*8, tileset), 
                        to=(nx + i*zoom, ny + j*zoom,
                            nx + i*zoom+(zoom), ny + j*zoom+(zoom)))
        return photo

    def getTkSubset(self, zoom, x, y, r):
        photo = tk.PhotoImage(width=8*r*zoom,
                              height=8*r*zoom)
        return self.drawTkSubset(photo, zoom, x, y, r, r, 0, 0)

    def get16x16tile(self, x, y, flip=False, block=True):
        output = Image.new('RGB', (16, 16))
        if block:
            draw = ImageDraw.Draw(output)
            draw.rectangle([0, 0, 16, 16], "#000000")
        for delx in range(0, 2):
            for dely in range(0, 2):
                loc = (x + delx, y + dely)
                if loc in self._tiles:
                    self._tiles[loc].draw(output, 1, self.palette, delx * 8, dely * 8)
        if (flip):
            return output.transpose(Image.FLIP_LEFT_RIGHT)
        return output

    def getStrip(self, height, block=True):
        max_ = self.getmaxytuple()
        rows = math.ceil((1+max_[1])/height)
        
        strip = Image.new('RGB', (rows*self.width*8, height*8))
        draw = ImageDraw.Draw(strip)
        if block:
            draw.rectangle([0, 0, rows*self.width*8,height*8], "#000000")
        for loc in self._tiles:
            y = loc[1] % height
            x = math.floor(loc[1] / height) * self.width + loc[0]
            x = x*8
            y = y*8
            self._tiles[loc].draw(strip, 1, self.palette, x, y)
        return strip

    def getmaxtuple(self):
        ys = [x[1] for x in self._tiles]
        xs = [x[0] for x in self._tiles if x[1] == max(ys)]
        return (max(xs), max(ys))
    
    def getmaxytuple(self):
        xs = [x[0] for x in self._tiles]
        ys = [x[1] for x in self._tiles if x[1] == max(xs)]
        return (max(xs), max(ys))

    def dump(self):
        output = {}
        output["version"] = 0.0 # just in case we need it later
        output["width"] = self.width
        output["height"] = self.height
        output["palette"] = self.palette

        output["tiles"] = []
        i = -1
        for page in self._pages:
            i = i+1
            output["tiles"].append({})
            for c in page:
                st = "".join((str(c[0]),",",str(c[1])))
                output["tiles"][i][st] = page[c].dump()

        return output

    def load(self, info):
        self._pages = []
        self.width = int(info["width"])
        self.height = int(info["height"])
        self.palette = [tuple(x) for x in info["palette"]]
        
        i = -1
        for page in info["tiles"]:
            i = i+1
            self._pages.append({})
            for tile in page:
                num = tile.split(",")
                if len(num) != 2:
                    continue
                loc = (int(num[0]),int(num[1]))
                self._pages[i][loc] = PixelTile().load(page[tile])

        self._tiles = self._pages[0]
        return self

    def changepage(self, pagenum):
        while len(self._pages) < (pagenum+1):
            self._pages.append({})
        self._tiles = self._pages[pagenum]

    def dellastpage(self):
        self._pages.pop()

    def ispageclear(self, num):
        return len(self._pages[num]) == 0

class PixelTile:
    def __init__(self, fill=0):
        self.width = 8
        self.height = 8
        self._pixels = [fill for x in range(self.width * self.height)]

    def get(self, x, y):
        return self._pixels[x + y*self.width]

    def set(self, x, y, val):
        i = x+y*self.width
        self._pixels[x + y*self.width] = val

    def flip(self, val1, val2):
        self._pixels = [val2 if x == val1 else x for x in self._pixels]

    def dump(self):
        return self._pixels

    def draw(self, photo, zoom, palette, x, y):
        for i in range(0, 8):
            for j in range(0, 8):
                if self.get(i,j) != 0:
                    photo.putpixel((x+i, y+j),
                         palette[self.get(i,j)])

    def load(self, pixels):
        self._pixels = pixels
        return self

class PixelSubset(PixelGrid):
    def __init__(self, parent, selection):
        self.parent = parent
        self.palette = parent.palette
        minx = min(selection[0], selection[2])
        maxx = max(selection[0], selection[2])
        miny = min(selection[1], selection[3])
        maxy = max(selection[1], selection[3])
        self.width = maxx-minx
        self.height = maxy-miny
        self._tiles = {}

        for i in range(0, parent.width):
            for j in range(0, parent.height):
                if i >= minx and i <= maxx and j >= miny and j <= maxy:
                    if (i,j) in parent._tiles:
                        self._tiles[(i-minx, j-miny)] = copy.deepcopy(
                                parent._tiles[(i,j)])

        self._pages = []
        self._pages.append(self._tiles)



