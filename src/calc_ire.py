#!/usr/bin/python2
import scipy.integrate as integrate 
from scipy import linalg
from scipy import pi 
import numpy as np

def read_center(idx):
    pntFile = open("./out/points_caucaia_" + str(idx))
    
    l = pntFile.readlines()[0].strip().split(' ')
    x_center = float(l[0])
    y_center = float(l[1])

    pntFile.close()

    return np.array([x_center, y_center])

def read_roads(idx):
    roadsFile = open("./out/chunks_caucaia_" + str(idx))
    lines = roadsFile.readlines()
    roads = []
    road = []
    for l in lines:
        l = l.strip()
        if len(l) == 0:
            roads.append(road)
            road = []
        else:
            d = l.split(' ')
            road.append(np.array([float(d[0]),float(d[1])]))

    roadsFile.close()

    return roads

def forest_function(R,A,B):
    return A*R + B

def h(R):
    K = 100
    A = 0.057
    B = 16.12
    return K/(2*pi*R*forest_function(R,A,B))

def param_line(t,start,end):
    return np.array([start[0] + t*(end[0]-start[0]),start[1] + t*(end[1]-start[1])])

def quad_linesegment(c,p1,p2):
    return integrate.quad(lambda t: h(linalg.norm(c-param_line(t,p1,p2))),0,1)

def ire_local(roads,point):
    error = 0
    total = 0
    length = 0
    for road in roads:
        length = length + road_length(road)
        p1 = road[0]        
        for p in road[1:]:
            (y,abserr) = quad_linesegment(point,p1,p)
            p1 = p
            total = total + y
            error = error + abserr

    avire = 0
    if length > 0:
        avire = total/length        
    
    return (total,avire,error)        

def road_length(r):
    p1 = r[0]
    length = 0
    for p in r[1:]:
        length = length + linalg.norm(p-p1)
        p1 = p
    return length

def ire(point_count):
    total = 0
    atotal = 0
    error = 0
    for i in range(0,(point_count-1)):
        center = read_center(i)
        roads = read_roads(i)
        if len(roads) > 0:
            (ti,ati,ei) = ire_local(roads,center)
            print "index ",i," - ire ",ti," - avire ", ati, " - error ", ei

            total = total + ti
            atotal = atotal + ati
            error = error + ei

    return (total,atotal,error)    

print ire(20)

