import tkinter as tk 

OBJECT_NAME = {
    "Photon": 0,
    "Antiphoton": 1,
}
NAME_OBJECT = {v: k for k, v in OBJECT_NAME.items()}

def TypeBox(window):
    var = tk.StringVar(window)
    return (tk.OptionMenu(window, var, *list(OBJECT_NAME.keys())), var)

def ObjectBox(root, onsave, _object):        
    x = _object.x 
    y = _object.y
    _type = _object.type 

    window = tk.Toplevel(root)

    xlabel = tk.Label(window, text="X:")
    xlabel.grid(row=0, column=0)
    xentry = tk.Entry(window, width=3)
    xentry.insert(0, x)
    xentry.grid(row=0, column=1)

    ylabel = tk.Label(window, text="Y:")
    ylabel.grid(row=1, column=0)
    yentry = tk.Entry(window, width=3)
    yentry.insert(0, y)
    yentry.grid(row=1, column=1)

    tk.Label(window, text="Type:").grid(row=2, column=0)
    (typeentry, typevar) = TypeBox(window)
    typevar.set(NAME_OBJECT[_type])
    typeentry.grid(row=2, column=1)

    def onclicksave():
        onsave(
            int(xentry.get()),
            int(yentry.get()),
            OBJECT_NAME[typevar.get()],
        )
        window.destroy()

    savebutton = tk.Button(window, text="Save", command=onclicksave)
    savebutton.grid(row=3, column=1)
