#!/usr/bin/python2
import scipy.integrate as integrate 
from scipy import linalg
from scipy import pi 
import numpy as np

config = dict()

def read_config():
    configFile = open("./cfg/config")
    lines = configFile.readlines()
    configFile.close()

    for l in lines:
        d = l.split('=')
        config[d[0].lower().strip()] = d[1].strip()
        
def read_centers(fname):
    pntFile = open(fname)  
    lines = pntFile.readlines()
    pntFile.close()

    points = []
    for l in lines:
        d = l.split(' ')
        points.append(np.array([float(d[0]),float(d[1])]))

    return points

def read_roads(fname):
    roadsFile = open(fname)
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
    return (A*R + B)

def h(R):
    K = float(config['k'])
    A = float(config['a'])
    B = float(config['b'])
    return K/(2*pi*R*forest_function(R,A,B))

def param_line(t,start,end):
    return np.array([start[0] + t*(end[0]-start[0]),start[1] + t*(end[1]-start[1])])

def quad_linesegment(c,p1,p2):
    return integrate.quad(lambda t: linalg.norm(np.array([p1[0]-p2[0],p1[1]-p2[1]]))*h(linalg.norm(c-param_line(t,p1,p2))),0,1)

def ire(roads,point):
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

    avire = total/length
    return (total,avire,error)        

def road_length(r):
    p1 = r[0]
    length = 0
    for p in r[1:]:
        length = length + linalg.norm(p-p1)
        p1 = p
    return length

def calculate_road_index(index_function):
    print "index,ire,avire,error"
    centers = read_centers(config['output_dir'] + "/points_" + config['output_prefix'] + "_all")
    for i in range(0,(len(centers)-1)):
        chunkfName = config['output_dir'] + "/chunks_" + config['output_prefix'] + "_" + str(i)
        roads = read_roads(chunkfName)
        if len(roads) > 0:
            (ti,ati,ei) = index_function(roads,centers[i])
            print i,",",ti,",",ati,",",ei

read_config()
calculate_road_index(ire)

