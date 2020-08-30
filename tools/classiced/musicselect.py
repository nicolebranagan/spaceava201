import tkinter as tk 

TRACKS = {
    "Spaceful": 0,
    "Impossibly Bossy": 1,
    "Your Soul is Lava": 2,
    "This Penance": 3,
    "Harsh Memories": 4
}

INV_TRACKS = {v: k for k, v in TRACKS.items()}

class MusicSelect(tk.Frame):
    def __init__(self, root, initial, onchange):
        tk.Frame.__init__(self, root)
        self.var = tk.StringVar(root)
        self.var.set(INV_TRACKS[initial])

        def callback(*args):
            onchange(TRACKS[self.var.get()])
        
        self.var.trace("w", callback)

        tk.Label(self, text="Music:").grid(row=0, column=0)

        optmenu = tk.OptionMenu(self, self.var, *list(TRACKS.keys()))
        optmenu.grid(row=0, column=1)

    def setValue(self, val):
        self.var.set(INV_TRACKS[val])
