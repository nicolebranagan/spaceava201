import math
import tkinter as tk
from tkinter import filedialog
from PIL import ImageTk, Image, ImageDraw
from tileset import getStarbase

class Application(tk.Frame):
    def __init__(self, master=None):
        tk.Frame.__init__(self, master)
        self.master = master
        self.pack()

        self.select = 0
        self.vramoffset = 0x2800

        tiles2x, tileset, tilesTk = getStarbase()
        self.tiles2x = tiles2x
        self.tileset = tileset
        self.tilesTk = tilesTk

        self.room = Room(self.tileset)

        self.createWidgets()
        self.drawroom()

    def createWidgets(self):
        self.tilecanvas = tk.Canvas(self, width=self.tiles2x.width,
                                    height=self.tiles2x.height)
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
        
        tk.Label(controls, text="VRAM offset").grid(row=1, column=0)
        self.vramentry = tk.Entry(controls, width=8)
        self.vramentry.insert(0, hex(self.vramoffset))
        self.vramentry.grid(row=1, column=1)

        def updatevram():
            self.vramoffset = int(self.vramentry.get(), 16)
        addenemybutton = tk.Button(controls, text="Update", command=updatevram)
        addenemybutton.grid(row=2, column=1)

        self.statusbar = tk.Label(self, text="Loaded successfully!", bd=1,
                                  relief=tk.SUNKEN, anchor=tk.W)
        self.statusbar.grid(row=2, column=0, columnspan=3, sticky=tk.W+tk.E)


    def drawroom(self):
        self.roomimg = self.room.draw()
        self.roomimgTk = ImageTk.PhotoImage(self.roomimg)
        self.viewcanvas.itemconfig(self.viewcanvasimage,
                                   image=self.roomimgTk)

    def tileclick(self, event):
        x = math.floor(self.tilecanvas.canvasx(event.x) / 32)
        #y = math.floor(self.tilecanvas.canvasy(event.y) / 32)

        self.select = x

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
                output.append(self.room.width)
                output.append(self.room.height)
                offsetbytes = hex(self.vramoffset)[2:]
                output.append(int(offsetbytes[:2], 16))
                output.append(int(offsetbytes[2:], 16))
                fileo.write(bytes(output))

    def open(self):
        filen = filedialog.askopenfilename(
                defaultextension=".bin",
                initialfile="classicdata.bin",
                initialdir="../../src/classicdata",
                filetypes=(("Binary files", "*.bin"),
                           ("All files", "*")),
                title="Open")
        if filen != () and filen != "":
            with open(filen, "rb") as fileo:
                self.room = Room.load(list(fileo.read(2048)), self.tileset)
            self.drawroom()

class Room:
    def __init__(self, tileset):
        self.width = 16
        self.height = 5
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
                image.paste(self.tileset[self.tiles[i]],(x*32, y*32))
                i = i+1
        return image

    @staticmethod
    def load(tiles, tileset):
        self = Room(tileset)
        self.tiles = tiles
        return self

root = tk.Tk()
app = Application(master=root)
app.mainloop()

