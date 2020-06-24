import math
import json
import tkinter as tk
from tkinter import filedialog
from PIL import ImageTk, Image
from pixelgrid import *

class Application(tk.Frame):
    def __init__(self, master=None):
        tk.Frame.__init__(self, master)
        self.master = master
        self.pack()

        self.select = 0

        pixelgrid = PixelGrid([(0,0,0)])
        with open("../../images/tiles/starbase.terra", "r") as fileo:
            pixelgrid.load(json.load(fileo))
        self.tiles = pixelgrid.getStrip(2)
        self.tiles2x = self.tiles.resize(
                (self.tiles.width * 2, self.tiles.height * 2),
                Image.NEAREST)
        self.tileset = self._getTileset()
        self.tilesTk = ImageTk.PhotoImage(self.tiles2x)

        self.room = Room(self.tileset)

        self.createWidgets()
        self.drawroom()

    def _getTileset(self):
        return [self.tiles.crop((x*16,0,(x+1)*16,16)) for x in
                range(0, int(self.tiles.width // 16))]

    def createWidgets(self):
        self.tilecanvas = tk.Canvas(self, width=self.tiles2x.width,
                                    height=self.tiles2x.height)
        self.tilecanvasimg = self.tilecanvas.create_image(
                0,0,anchor=tk.NW,image=self.tilesTk)
        self.tilecanvas.grid(row=0, column=0, columnspan = 3)
        self.tilecanvas.bind("<Button-1>", self.tileclick)
        
        self.viewcanvas = tk.Canvas(self, width=self.room.width*16, height=self.room.height*16)
        self.viewcanvas.grid(row=1, column=1)
        self.viewcanvasimage = self.viewcanvas.create_image(0,0,anchor=tk.NW)
        self.viewcanvas.bind("<Button-1>", self.viewclick)
        self.viewcanvas.bind("<B1-Motion>", self.viewclick)
        self.viewcanvas.bind("<Motion>", self.viewmove)
        self.viewcanvas.bind("<Button-2>", self.cviewclick)
        self.viewcanvas.bind("<Button-3>", self.rviewclick)
        controls = tk.Frame(self, width=6*32, height=6*32)
        controls.grid(row=1, column=2)
        loadbutton = tk.Button(controls, text="Open", command=self.open)
        loadbutton.grid(row=0, column=0)
        savebutton = tk.Button(controls, text="Save", command=self.save)
        savebutton.grid(row=0, column=1)

        self.objectlist = tk.Listbox(controls)
        self.objectlist.grid(row=1, column=0, columnspan=2)
        self.xentry = tk.Entry(controls, width=3)
        self.xentry.insert(0, "0")
        self.xentry.grid(row=2, column=0)
        self.yentry = tk.Entry(controls, width=3)
        self.yentry.insert(0, "0")
        self.yentry.grid(row=2, column=1)

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
        clickX = math.floor(self.viewcanvas.canvasx(event.x) / 16)
        clickY = math.floor(self.viewcanvas.canvasy(event.y) / 16)
        
        if self.room.get(clickX, clickY) != self.select:
            self.room.set(clickX, clickY, self.select)
            self.drawroom()

    def cviewclick(self, event):
        clickX = math.floor(self.viewcanvas.canvasx(event.x) / 16)
        clickY = math.floor(self.viewcanvas.canvasy(event.y) / 16)
        self.xentry.delete(0, tk.END)
        self.xentry.insert(0, str(clickX))
        self.yentry.delete(0, tk.END)
        self.yentry.insert(0, str(clickY))

    def rviewclick(self, event):
        clickX = math.floor(self.viewcanvas.canvasx(event.x) / 16)
        clickY = math.floor(self.viewcanvas.canvasy(event.y) / 16)
        self.select = self.room.get(clickX, clickY)

    def viewmove(self, event):
        clickX = math.floor(self.viewcanvas.canvasx(event.x) / 16)
        clickY = math.floor(self.viewcanvas.canvasy(event.y) / 16)
        
        self.statusbar.config(
                text="Coordinates: {}, {}".format(clickX, clickY))

    def save(self):
        filen = filedialog.asksaveasfilename(
                defaultextension=".bin",
                initialfile="classicdata.bin",
                initialdir="../../src/classicdata",
                filetypes=(("Binary files", "*.bin"),
                           ("All files", "*")),
                title="Save")
        if filen != () and filen != "":
            with open(filen, "wb") as fileo:
                fileo.write(bytes(self.room.tiles))

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
        self.width = 64
        self.height = 32
        self.tileset = tileset
        self.tiles = [0 for x in range(0,self.width*self.height)]
        self.objects = []

    def set(self, x, y, v):
        if x >= self.width or y >= self.height or x < 0 or y < 0:
            return
        self.tiles[x + y*self.width] = v

    def get(self, x, y):
        if x >= self.width or y >= self.height or x < 0 or y < 0:
            return 0
        return self.tiles[x + y*self.width]

    def draw(self):
        image = Image.new("RGB",(self.width*16, self.height*16))
        i = 0
        for y in range(0, self.height):
            for x in range(0, self.width):
                image.paste(self.tileset[self.tiles[i]],(x*16, y*16))
                i = i+1
        return image

    def dump(self):
        return {"tiles": self.tiles}

    @staticmethod
    def load(tiles, tileset):
        self = Room(tileset)
        self.tiles = tiles
        return self

root = tk.Tk()
app = Application(master=root)
app.mainloop()

