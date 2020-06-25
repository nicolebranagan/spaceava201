import tkinter as tk 

def EnemyBox(root, onsave, enemy=None):        
        if (enemy is not None):
            x = enemy.x 
            y = enemy.y
            _type = enemy.type 
            facing = enemy.facing 
            delx = enemy.delx
            dely = enemy.dely
        else:
            x = 0
            y = 0
            _type = 0
            facing = 0
            delx = 0
            dely = 0

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
        typeentry = tk.Entry(window, width=3)
        typeentry.insert(0, _type)
        typeentry.grid(row=2, column=1)

        tk.Label(window, text="Facing:").grid(row=3, column=0)
        faceentry = tk.Entry(window, width=3)
        faceentry.insert(0, facing)
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
                int(typeentry.get()),
                int(faceentry.get()),
                int(delxentry.get()),
                int(delyentry.get())
            )
            window.destroy()

        savebutton = tk.Button(window, text="Save", command=onclicksave)
        savebutton.grid(row=6, column=1)
