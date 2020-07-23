import tkinter as tk 

TILESETS = {
    "Spaceful": 0,
    "Betelgeuse": 1
}

INV_TILESETS = {v: k for k, v in TILESETS.items()}

class TileSelect(tk.Frame):
    def __init__(self, root, initial, onchange):
        tk.Frame.__init__(self, root)
        self.var = tk.StringVar(root)
        self.var.set(INV_TILESETS[initial])

        def callback(*args):
            onchange(TILESETS[self.var.get()])
        
        self.var.trace("w", callback)

        tk.Label(self, text="Graphics:").grid(row=0, column=0)

        optmenu = tk.OptionMenu(self, self.var, *list(TILESETS.keys()))
        optmenu.grid(row=0, column=1)

    def setValue(self, val):
        self.var.set(INV_TILESETS[val])
