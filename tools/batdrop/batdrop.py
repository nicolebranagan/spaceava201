import math
import tkinter as tk
from tkinter import filedialog
from PIL import ImageTk, Image, ImageDraw
from tileset import getTilesets, TILESET_NAME, NAME_TILESET, MAX_TILESET_WIDTH

DEFAULT_WIDTH = 16
DEFAULT_HEIGHT = 5

def TilesetBox(window):
    var = tk.StringVar(window)
    return (tk.OptionMenu(window, var, *list(TILESET_NAME.keys())), var)

class Application(tk.Frame):
    def __init__(self, master=None):
        tk.Frame.__init__(self, master)
        self.master = master
        self.pack()

        self.select = 0
        self.vramoffset = 0x1000

        tiles2x, tileset, tilesTk = tilesets[0]
        self.tiles2x = tiles2x
        self.tileset = tileset
        self.tilesTk = tilesTk

        self.room = Room(0, DEFAULT_WIDTH, DEFAULT_HEIGHT)

        self.createWidgets()
        self.drawroom()

    def createWidgets(self):
        self.tilecanvas = tk.Canvas(self, width=self.tilesTk.width(),
                                    height=self.tilesTk.height())
        self.tilecanvasimg = self.tilecanvas.create_image(
                0,0,anchor=tk.NW,image=self.tilesTk)
        self.tilecanvas.grid(row=0, column=0, columnspan = 3)
        self.tilecanvas.bind("<Button-1>", self.tileclick)
        
        self.viewcanvas = tk.Canvas(self, width=self.room.width*32, height=self.room.height*32)
        self.viewcanvas.grid(row=1, column=1)
        self.viewcanvasimage = self.viewcanvas.create_image(0,0,anchor=tk.NW)
        self.viewcanvas.bind("<Button-1>", self.viewclick)
        self.viewcanvas.bind("<B1-Motion>", self.viewclick)
        self.viewcanvas.bind("<Motion>", self.viewmove)
        self.viewcanvas.bind("<Button-2>", self.rviewclick)
        self.viewcanvas.bind("<Button-3>", self.rviewclick)
        controls = tk.Frame(self, width=6*32, height=6*32)
        controls.grid(row=1, column=2)
        loadbutton = tk.Button(controls, text="Open", command=self.open)
        loadbutton.grid(row=0, column=0)
        savebutton = tk.Button(controls, text="Save", command=self.save)
        savebutton.grid(row=0, column=1)

        (starbaseentry, starbasevar) = TilesetBox(controls)
        self.starbasevar = starbasevar
        starbaseentry.grid(row=1, column=0, columnspan=2)
        self.starbasevar.set(NAME_TILESET[self.room.tileset])
        self.starbasevar.trace("w", 
            lambda *_: self.changetileset(TILESET_NAME[self.starbasevar.get()])
        )
        
        tk.Label(controls, text="VRAM offset").grid(row=2, column=0)
        self.vramentry = tk.Entry(controls, width=8)
        self.vramentry.insert(0, hex(self.vramoffset))
        self.vramentry.grid(row=2, column=1)

        def updatevram():
            self.vramoffset = int(self.vramentry.get(), 16)
        addenemybutton = tk.Button(controls, text="Update", command=updatevram)
        addenemybutton.grid(row=3, column=1)
        
        self.xentry = tk.Entry(controls, width=4)
        self.xentry.insert(0, self.room.width)
        self.xentry.grid(row=4, column=0)

        self.yentry = tk.Entry(controls, width=4)
        self.yentry.insert(0, self.room.height)
        self.yentry.grid(row=4, column=1)

        def resetroom():
            self.room = Room(0, int(self.xentry.get()), int(self.yentry.get()))
            self.viewcanvas.config(
                width=self.room.width*32, height=self.room.height*32
            )
            self.changetileset(0)
            self.drawroom()
            
        resetbutton = tk.Button(controls, text="Reset", command=resetroom)
        resetbutton.grid(row=5, column=0, columnspan=2)

        resizebutton = tk.Button(controls, text="Resize", command=self.resizeroom)
        resizebutton.grid(row=6, column=0, columnspan=2)

        self.statusbar = tk.Label(self, text="Loaded successfully!", bd=1,
                                  relief=tk.SUNKEN, anchor=tk.W)
        self.statusbar.grid(row=2, column=0, columnspan=3, sticky=tk.W+tk.E)

    def changetileset(self, index):
        tiles2x, tileset, tilesTk = tilesets[index]
        self.tiles2x = tiles2x
        self.tileset = tileset
        self.tilesTk = tilesTk

        self.room.tileset = index
        self.tilecanvas.config(
            width=self.tilesTk.width(), height=self.tilesTk.height()
        )
        self.tilecanvasimg = self.tilecanvas.create_image(
                0,0,anchor=tk.NW,image=self.tilesTk)
        self.drawroom()

    def drawroom(self):
        self.roomimg = self.room.draw()
        self.roomimgTk = ImageTk.PhotoImage(self.roomimg)
        self.viewcanvas.itemconfig(self.viewcanvasimage,
                                   image=self.roomimgTk)

    def tileclick(self, event):
        x = math.floor(self.tilecanvas.canvasx(event.x) / 32)
        y = math.floor(self.tilecanvas.canvasy(event.y) / 32)

        self.select = x + (y * MAX_TILESET_WIDTH)

    def viewclick(self, event):
        clickX = math.floor(self.viewcanvas.canvasx(event.x) / 32)
        clickY = math.floor(self.viewcanvas.canvasy(event.y) / 32)

        if self.room.get(clickX, clickY) != self.select:
            self.room.set(clickX, clickY, self.select)
            self.drawroom()

    def rviewclick(self, event):
        clickX = math.floor(self.viewcanvas.canvasx(event.x) / 32)
        clickY = math.floor(self.viewcanvas.canvasy(event.y) / 32)
        self.select = self.room.get(clickX, clickY)

    def viewmove(self, event):
        clickX = math.floor(self.viewcanvas.canvasx(event.x) / 32)
        clickY = math.floor(self.viewcanvas.canvasy(event.y) / 32)
        
        self.statusbar.config(
                text="Coordinates: {}, {}".format(clickX, clickY))

    def resizeroom(self):
        x = int(self.xentry.get())
        y = int(self.yentry.get())

        newtiles = [0 for _ in range(0, x * y)]
        for i in range(0, self.room.width):
            for j in range(0, self.room.height):
                if (i + (x * (j + 1))) < (x*y):
                    newtiles[i + (x * (j + 1))] = self.room.get(i, j)
        self.room = Room(0, x, y)
        self.viewcanvas.config(
            width=self.room.width*32, height=self.room.height*32
        )
        self.room.tiles = newtiles
        self.drawroom()

    def save(self):
        filen = filedialog.asksaveasfilename(
                defaultextension=".bin",
                initialfile="bat.bin",
                initialdir="../../src/bats",
                filetypes=(("Binary files", "*.bin"),
                           ("All files", "*")),
                title="Save")
        if filen != () and filen != "":
            with open(filen, "wb") as fileo:
                tiles_by_offset = [
                    0 for _ in range(0, len(self.room.tiles) * 4)
                ]
                for y in range(0, self.room.height):
                    for x in range(0, self.room.width):
                            tile = self.room.get(x, y) * 4
                            tiles_by_offset[
                                ((y * 2) * self.room.width * 2) + (x * 2)
                            ] = tile
                            tiles_by_offset[
                                ((y * 2) * self.room.width * 2) + (x * 2) + 1
                            ] = tile + 1
                            tiles_by_offset[
                                (((y * 2) + 1) * self.room.width * 2) + (x * 2)
                            ] = tile + 2
                            tiles_by_offset[
                                (((y * 2) + 1) * self.room.width * 2) + (x * 2) + 1
                            ] = tile + 3
                tile_base = self.vramoffset // 16
                output = []
                def to_binary(input):
                    return bin(input)[2:].zfill(12) # strip 0b and make 12 digits
                for tile in tiles_by_offset:
                    index = to_binary(tile_base + tile)
                    output.append(int(f'{index[4:]}', 2))
                    output.append(int(f'0001{index[0:4]}', 2))
                output.append(self.room.tileset)
                output.append(self.room.width)
                output.append(self.room.height)
                offsetbytes = hex(self.vramoffset)[2:]
                output.append(int(offsetbytes[:2], 16))
                output.append(int(offsetbytes[2:], 16))
                fileo.write(bytes(output))

    def open(self):
        filen = filedialog.askopenfilename(
                defaultextension=".bin",
                initialfile="starbase.bin",
                initialdir="../../src/bats",
                filetypes=(("Binary files", "*.bin"),
                           ("All files", "*")),
                title="Open")
        if filen != () and filen != "":
            with open(filen, "rb") as fileo:
                data = fileo.read()
            signature = data[(len(data)-5):]
            vram_offset = int(
                f'{hex(signature[3])[2:].zfill(2)}{hex(signature[4])[2:].zfill(2)}',
                16
            )
            tiledata = data[:(len(data)-5)]
            tiles = []
            tileset = signature[0]
            width = signature[1]
            height = signature[2]
            for y in range (0, height):
                for x in range(0, width):
                    topleft = 2*((y*4*width)+(2*x))
                    binary1 = bin(tiledata[topleft])[2:].zfill(8)
                    binary2 = bin(tiledata[topleft + 1])[2:].zfill(8)
                    pointer = int(f'{binary2[4:]}{binary1}', 2)
                    tile_base = vram_offset // 16
                    tiles.append((pointer - tile_base) // 4)
            self.room = Room.load(tiles, tileset, width, height)
            self.vramoffset = vram_offset

            self.viewcanvas.config(
                width=self.room.width*32, height=self.room.height*32
            )
            self.vramentry.delete(0, tk.END)
            self.vramentry.insert(0, hex(vram_offset))
            self.xentry.delete(0, tk.END)
            self.xentry.insert(0, width)
            self.yentry.delete(0, tk.END)
            self.yentry.insert(0, height)   
            self.starbasevar.set(NAME_TILESET[self.room.tileset])

            self.changetileset(tileset)
            self.drawroom()

class Room:
    def __init__(self, tileset, width, height):
        self.width = width
        self.height = height
        self.tileset = tileset
        self.tiles = [0 for x in range(0,self.width*self.height)]

    def set(self, x, y, v):
        if x >= self.width or y >= self.height or x < 0 or y < 0:
            return
        self.tiles[x + y*self.width] = v

    def get(self, x, y):
        if x >= self.width or y >= self.height or x < 0 or y < 0:
            return 0
        return self.tiles[x + y*self.width]

    def draw(self):
        image = Image.new("RGB",(self.width*32, self.height*32))
        i = 0
        for y in range(0, self.height):
            for x in range(0, self.width):
                image.paste(tilesets[self.tileset][1][self.tiles[i]],(x*32, y*32))
                i = i+1
        return image

    @staticmethod
    def load(tiles, tileset, width, height):
        self = Room(tileset, width, height)
        self.tiles = tiles
        self.width = width
        self.height = height
        return self

root = tk.Tk()
tilesets = getTilesets()
app = Application(master=root)
app.mainloop()

