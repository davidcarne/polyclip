#!/usr/bin/python

from Tkinter import *
import math
import lib.python_wrapper as python_wrapper

size = 4

id_vertex_map = {}
id_center_map = {}

class Vertex(object):
    def __init__(self, x, y, canvas, poly, color1, color2):
        self.x = x
        self.y = y
        self.poly = poly
        self.linetonext_n = canvas.create_line(x,y,x,y)
        self.handle_n = canvas.create_rectangle(x-size, y-size, x+size, y+size, outline="", fill=color1)
        self.center_n = canvas.create_rectangle(x-size, y-size, x+size, y+size, outline="", fill=color2)
    
        id_vertex_map[self.handle_n] = self
        id_center_map[self.center_n] = self
        
    def updatePosition(self,x,y):
        self.x = x
        self.y = y
        self.poly.canvas.coords(self.handle_n, self.x-size, self.y-size, self.x+size, self.y+size)
        self.poly.updateLines()
        
class Polygon(object):
    def __init__(self, canvas, color1, color2):
        self.color1 = color1
        self.color2 = color2
        self.verticies = []
        
        self.canvas = canvas
        
    def addPoint(self, x, y):
        self.verticies.append(Vertex(x,y,self.canvas, self,self.color1, self.color2))
        self.updateLines();
    
    def insertPoint(self, after, x, y):
        p = self.verticies.index(after)+1
        v = Vertex(x,y,self.canvas, self,self.color1, self.color2)
        self.verticies.insert(p,v)
        self.updateLines();
        return v

    def updateLines(self):
        for i in xrange(len(self.verticies)):
            a = self.verticies[i]
            b = self.verticies[(i + 1) % len(self.verticies)]
            
            self.canvas.coords(a.linetonext_n, a.x, a.y, b.x, b.y)
            c_x = a.x/2+b.x/2
            c_y = a.y/2+b.y/2
            self.canvas.coords(a.center_n, c_x-size, c_y-size, c_x+size, c_y+size)
    
master = Tk()

def grabbed_center(v):
    global sel_hand
    ax, ay, bx, by = v.poly.canvas.coords(v.center_n);
    n = v.poly.insertPoint(v, ax/2+bx/2, ay/2+by/2)
    sel_hand = n

sel_hand = None

def grabbed_handle(v):
    global sel_hand
    sel_hand = v
    
def movecb(event):
    canvas = event.widget
    x = canvas.canvasx(event.x)
    y = canvas.canvasy(event.y)
    if sel_hand:
        sel_hand.updatePosition(x,y)
    
def press_callback(event):
    canvas = event.widget
    x = canvas.canvasx(event.x)
    y = canvas.canvasy(event.y)
    i, = canvas.find_closest(x, y)
    if canvas.type(i) != "rectangle": return;
    
    ax,ay,bx,by = canvas.coords(i)
    cx = ax/2 + bx/2
    cy = ay/2 + by/2
    d = math.sqrt((cx - x)**2 + (cy-y)**2)
    if d > 6:
        return
    
    if i in id_center_map:
        grabbed_center(id_center_map[i])
    elif i in id_vertex_map:
        grabbed_handle(id_vertex_map[i])
        

def rel_cb(event):
    
    global sel_hand
    sel_hand = None
    
w = Canvas(master, width=600, height=600)
w.bind("<Button-1>", press_callback)
w.bind("<ButtonRelease-1>", rel_cb)

w.bind("<B1-Motion>", movecb)

w.pack(side=LEFT)

subject = Polygon(w, "#cc0000", "#ffaaaa")

subject.addPoint(10,10)
subject.addPoint(295,10)
subject.addPoint(295,550)
subject.addPoint(10,550)

clip = Polygon(w, "#0000cc", "#aaaaff")

clip.addPoint(305,10)
clip.addPoint(550,10)
clip.addPoint(550,550)
clip.addPoint(305,550)
#w.create_line(0, 0, 200, 100)
#w.create_line(0, 100, 200, 0, fill="red", dash=(4, 4))

#w.create_rectangle(50, 25, 150, 75, fill="blue", outline="")
f = Frame(master, width=30);
f.pack(side=RIGHT)

b = Button(f, text="Clip",width=20)
b.pack()
c = Button(f, text="Clear",width=20)
c.pack()

clipPolys = None

def clipCB(event):
    global clipPolys
    if clipPolys:
        clearCB(None)
        
    po = python_wrapper.booleanOp(subject.verticies, clip.verticies, python_wrapper.OP_INTERSECT)
    clipPolys = []
    for i in po:
        coords = []
        for j in i:
            coords.extend(j)
    
        clipPolys.append(w.create_polygon(*coords, fill="#dddddd"))
    
    
def clearCB(event):
    global clipPolys
    if not clipPolys:
        return
    print "clearing"
    for i in clipPolys:
        w.delete(i)
    clipPoly = None

b.bind("<Button-1>", clipCB)
c.bind("<Button-1>", clearCB)

mainloop()
