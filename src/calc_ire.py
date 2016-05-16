#!/usr/bin/python2
import sys

import numpy as np

from ire import IRE
from forestindex import ForestIndex

#local configuration
import config

cfg = dict()

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

if len(sys.argv) > 1:
    cfg = config.read_config(sys.argv[1])
else:
    cfg = config.read_config(config.DEFAULT_CONFIG_FILE)

centers = read_centers(cfg['output_dir'] + "/points_" + cfg['output_prefix'] + "_all")
roads_list = []
for i in range(0,len(centers)):
    chunkfName = cfg['output_dir'] + "/chunks_" + cfg['output_prefix'] + "_" + str(i)
    roads = read_roads(chunkfName)
    roads_list.append(roads)

ire = IRE(float(cfg['a']),float(cfg['b']),float(cfg['k']))
fidx = ForestIndex(ire,roads_list,centers)

res = fidx.calculate_index()

print "idx,ire,avire,error"
for r in res:
    print r[0],",",r[1],",",r[2],",",r[3]
