import math
import tkinter as tk
from tkinter import filedialog
from PIL import ImageTk, Image, ImageDraw
from enemybox import EnemyBox
from objectbox import ObjectBox
from tileset import getStarbase

class Application(tk.Frame):
    def __init__(self, master=None):
        tk.Frame.__init__(self, master)
        self.master = master
        self.pack()

        self.select = 0
        self.savedx = 0
        self.savedy = 0

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
        
        self.setstartmode = False
        setstartbutton = tk.Button(controls, text="Set Start", command=self.setstart)
        setstartbutton.grid(row=3, column=0, columnspan=2)

        def onclickaddenemy():
            def onsave(x, y, type, facing, delx, dely):
                self.room.enemies.append(Enemy(x, y, type, facing, delx, dely))
                self.drawroom()
            EnemyBox(root, onsave, Enemy(self.savedx, self.savedy, 0, 0, 0, 0))

        addenemybutton = tk.Button(controls, text="Add Enemy", command=onclickaddenemy)
        addenemybutton.grid(row=4, column=0, columnspan=2)

        def onclickaddobject():
            def onsave(x, y, type):
                self.room.objects.append(Object(x, y, type))
                self.drawroom()
            ObjectBox(root, onsave, Object(self.savedx, self.savedy, 0))

        addobjectbutton = tk.Button(controls, text="Add Object", command=onclickaddobject)
        addobjectbutton.grid(row=5, column=0, columnspan=2)

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

        if (self.setstartmode):
            self.setstartmode = False 
            self.room.startx = clickX
            self.room.starty = clickY 
            self.drawroom()
            return 

        if self.room.get(clickX, clickY) != self.select:
            self.room.set(clickX, clickY, self.select)
            self.drawroom()

    def cviewclick(self, event):
        clickX = math.floor(self.viewcanvas.canvasx(event.x) / 16)
        clickY = math.floor(self.viewcanvas.canvasy(event.y) / 16)

        for idx, enem in enumerate(self.room.enemies):
            if (enem.x != clickX or enem.y != clickY):
                continue
            def onsave(x, y, type, facing, delx, dely):
                self.room.enemies[idx] = Enemy(x, y, type, facing, delx, dely)
                self.drawroom()
            def ondelete():
                self.room.enemies.pop(idx)
                self.drawroom()
            EnemyBox(root, onsave, enem, onsave)
            return
        for idx, obj in enumerate(self.room.objects):
            if (obj.x != clickX or obj.y != clickY):
                continue
            def onsave(x, y, type):
                self.room.objects[idx] = Object(x, y, type)
                self.drawroom()
            def ondelete():
                self.room.objects.pop(idx)
                self.drawroom()
            ObjectBox(root, onsave, obj, ondelete)
            return
        self.savedx = clickX 
        self.savedy = clickY

    def rviewclick(self, event):
        clickX = math.floor(self.viewcanvas.canvasx(event.x) / 16)
        clickY = math.floor(self.viewcanvas.canvasy(event.y) / 16)
        self.select = self.room.get(clickX, clickY)

    def viewmove(self, event):
        clickX = math.floor(self.viewcanvas.canvasx(event.x) / 16)
        clickY = math.floor(self.viewcanvas.canvasy(event.y) / 16)
        
        self.statusbar.config(
                text="Coordinates: {}, {}".format(clickX, clickY))

    def setstart(self):
        self.setstartmode = True

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
                fileo.write(self.room.dump())

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
                self.room.startx = fileo.read(1)[0]
                self.room.starty = fileo.read(1)[0]
                while True:
                    xbyte = fileo.read(1)
                    if (xbyte[0] == 255):
                        break
                    self.room.enemies.append(Enemy(
                        xbyte[0],
                        fileo.read(1)[0],
                        fileo.read(1)[0],
                        fileo.read(1)[0],
                        fileo.read(1)[0],
                        fileo.read(1)[0]
                    ))
                while True:
                    xbyte = fileo.read(1)
                    if (xbyte[0] == 255):
                        break
                    self.room.objects.append(Object(
                        xbyte[0],
                        fileo.read(1)[0],
                        fileo.read(1)[0]
                    ))
            self.drawroom()

class Room:
    def __init__(self, tileset):
        self.width = 64
        self.height = 32
        self.tileset = tileset
        self.tiles = [0 for x in range(0,self.width*self.height)]
        self.objects = []
        self.startx = 0
        self.starty = 0
        self.enemies = []

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
        draw = ImageDraw.Draw(image)
        draw.text((self.startx*16 + 4, self.starty*16 + 4), "S", (255, 0, 0))
        for enem in self.enemies:
            draw.text((enem.x*16 + 4, enem.y*16 + 4), "E", (0, 128, 0))
        for obj in self.objects:
            draw.text((obj.x*16 + 4, obj.y*16 + 4), "O", (0, 0, 255))
        return image

    def dump(self):
        objects = b''
        for obj in self.objects:
            objects = objects + obj.dump()
        enemies = b''
        for enem in self.enemies:
            enemies = enemies + enem.dump()
        data = bytes(self.tiles) + bytes([self.startx, self.starty]) + enemies + bytes([255]) + objects  
        # Ensure all data is sector-aligned
        return data + bytes([255 for _ in range(0, 4096 - len(data))])

    @staticmethod
    def load(tiles, tileset):
        self = Room(tileset)
        self.tiles = tiles
        return self

class Object:
    def __init__(self, x, y, type):
        self.x = x
        self.y = y
        self.type = type 
    
    def dump(self):
        return bytes([self.x, self.y, self.type])

class Enemy:
    def __init__(self, x, y, type, facing, delx, dely):
        self.x = x
        self.y = y
        self.type = type 
        self.facing = facing 
        self.delx = delx 
        self.dely = dely
    
    def dump(self):
        return bytes([self.x, self.y, self.type, self.facing, self.delx, self.dely])

root = tk.Tk()
app = Application(master=root)
app.mainloop()

