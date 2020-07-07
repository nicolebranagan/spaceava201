import tkinter as tk 

DIRECTIONS = {
    "UP": 0,
    "DOWN": 1,
    "LEFT": 2,
    "RIGHT": 3,
}
INV_DIRECTION = {v: k for k, v in DIRECTIONS.items()}

def DirectionBox(window):
    var = tk.StringVar(window)
    return (tk.OptionMenu(window, var, *list(DIRECTIONS.keys())), var)

ENEMY_NAME = {
    "Bigmouth": 0,
    "Ball": 1,
    "Eyewalk": 2
}
NAME_ENEMY = {v: k for k, v in ENEMY_NAME.items()}

def TypeBox(window):
    var = tk.StringVar(window)
    return (tk.OptionMenu(window, var, *list(ENEMY_NAME.keys())), var)

def EnemyBox(root, onsave, enemy, ondelete=None):        
    x = enemy.x 
    y = enemy.y
    _type = enemy.type 
    facing = enemy.facing 
    delx = enemy.delx
    dely = enemy.dely

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
    typevar.set(NAME_ENEMY[_type])
    typeentry.grid(row=2, column=1)

    tk.Label(window, text="Facing:").grid(row=3, column=0)
    (faceentry, facevar) = DirectionBox(window)
    facevar.set(INV_DIRECTION[facing])
    faceentry.grid(row=3, column=1)

    tk.Label(window, text="Delx:").grid(row=4, column=0)
    delxentry = tk.Entry(window, width=3)
    delxentry.insert(0, delx)
    delxentry.grid(row=4, column=1)

    tk.Label(window, text="Dely:").grid(row=5, column=0)
    delyentry = tk.Entry(window, width=3)
    delyentry.insert(0, dely)
    delyentry.grid(row=5, column=1)

    def onclicksave():
        onsave(
            int(xentry.get()),
            int(yentry.get()),
            ENEMY_NAME[typevar.get()],
            DIRECTIONS[facevar.get()],
            int(delxentry.get()),
            int(delyentry.get())
        )
        window.destroy()

    savebutton = tk.Button(window, text="Save", command=onclicksave)
    savebutton.grid(row=6, column=1)

    if (ondelete is not None):
        def onclickdelete():
            ondelete()
            window.destroy()
            
        deletebutton = tk.Button(window, text="Delete", command=onclickdelete)
        deletebutton.grid(row=7, column=1)