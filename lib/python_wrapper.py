import ctypes

lpc = ctypes.CDLL("build/libPolyClip.dylib")

PC_polyPoint = lpc.PC_polyPoint
PC_polyPoint.restype = ctypes.c_void_p
PC_polyPoint.argtypes = [ctypes.c_void_p, ctypes.c_double, ctypes.c_double]


PC_getPolyPoint = lpc.PC_getPolyPoint
PC_getPolyPoint.restype = ctypes.c_void_p
PC_getPolyPoint.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_double)]

class PC_polygon_ll(ctypes.Structure):
    pass

PC_polygon_ll._fields_ = [ ("next", ctypes.POINTER(PC_polygon_ll)),
                 ("firstv", ctypes.c_void_p)
                 ]

PC_polygon_boolean = lpc.PC_polygon_boolean
PC_polygon_boolean.restype = ctypes.POINTER(PC_polygon_ll)
PC_polygon_boolean.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_int]


OP_SUBTRACT = 0
OP_INTERSECT = 1
OP_UNION = 2
OP_XOR = 3


def createPCPolygon(x):
    phandle = None
    p = None
    for i in x:
        p = PC_polyPoint(p, i.x, i.y)
        if not phandle:
            phandle = p
    return phandle
    
    
def breakoutPCPolygon(p):
    x = ctypes.c_double()
    y = ctypes.c_double()
    l = []
    while 1:
        p = PC_getPolyPoint(p, ctypes.byref(x), ctypes.byref(y))
        l.append((float(x.value), float(y.value)))
        if not p: break;
    return l
    

def booleanOp(la, lb, op):
    pa = createPCPolygon(la)
    pb = createPCPolygon(lb)
    ps = PC_polygon_boolean(pa, pb, op)
    
    l = []
    while 1:
        if not ps: break
        po = ps.contents.firstv
    
        l.append(breakoutPCPolygon(po))
        ps = ps.contents.next
    return l