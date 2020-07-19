import math
import tkinter as tk
from tkinter import filedialog
from PIL import ImageTk, Image, ImageDraw
from tileset import getTileset

DEFAULT_WIDTH = 9
DEFAULT_HEIGHT = 10

class Application(tk.Frame):
    def __init__(self, master=None):
        tk.Frame.__init__(self, master)
        self.master = master
        self.pack()

        self.select = 0

        tiles2x, _tileset, tilesTk = tileset
        self.tiles2x = tiles2x
        self.tilesTk = tilesTk

        self.room = Room(DEFAULT_WIDTH+1, DEFAULT_HEIGHT)

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
                initialfile="mirrordata.bin",
                initialdir="../../src/mirrordata",
                filetypes=(("Binary files", "*.bin"),
                           ("All files", "*")),
                title="Save")
        if filen != () and filen != "":
            with open(filen, "wb") as fileo:
                tiles = [0 for _ in range(0, DEFAULT_WIDTH * DEFAULT_HEIGHT)]
                palette = [0 for _ in range(0, DEFAULT_HEIGHT - 1)]
                for y in range(0, DEFAULT_HEIGHT):
                    for x in range(0, DEFAULT_WIDTH):
                        tiles[(y * DEFAULT_WIDTH) + x] = self.room.get(x, y)
                for i in range(0, DEFAULT_HEIGHT - 1):
                    palette[i]  = self.room.get(DEFAULT_WIDTH, i)
                output = tiles + palette
                fileo.write(bytes(output))

    def open(self):
        filen = filedialog.askopenfilename(
                defaultextension=".bin",
                initialfile="mirrordata.bin",
                initialdir="../../src/mirrordata",
                filetypes=(("Binary files", "*.bin"),
                           ("All files", "*")),
                title="Save")
        if filen != () and filen != "":
            with open(filen, "rb") as fileo:
                data = fileo.read()
            grid = data[0:(DEFAULT_WIDTH * DEFAULT_HEIGHT)]
            palette = data[
                (DEFAULT_WIDTH * DEFAULT_HEIGHT):
                ((DEFAULT_WIDTH * DEFAULT_HEIGHT)+DEFAULT_HEIGHT)
            ]
            tiles = [0 for _ in range(0, (DEFAULT_WIDTH + 1) * (DEFAULT_HEIGHT))]
            for y in range(0, DEFAULT_HEIGHT):
                for x in range(0, DEFAULT_WIDTH + 1):
                    if x == DEFAULT_WIDTH and y < (DEFAULT_HEIGHT - 1):
                        tiles[x + (y * (DEFAULT_WIDTH + 1))] = palette[y]
                    elif x < DEFAULT_WIDTH:
                        tiles[x + (y * (DEFAULT_WIDTH + 1))] = grid[x + (y * DEFAULT_WIDTH)]
            self.room = Room.load(tiles, DEFAULT_WIDTH+1, DEFAULT_HEIGHT)
            self.drawroom()

class Room:
    def __init__(self, width, height):
        self.width = width
        self.height = height
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
                image.paste(tileset[1][self.tiles[i]],(x*32, y*32))
                i = i+1
        return image

    @staticmethod
    def load(tiles, width, height):
        self = Room(width, height)
        self.tiles = tiles
        self.width = width
        self.height = height
        return self

root = tk.Tk()
tileset = getTileset()
app = Application(master=root)
app.mainloop()

