#!/usr/bin/python2
from osgeo import ogr
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.lines as lines

root = './data/gis/'
roadfile = root + 'estradas_Macacu.shp'
pointsfile = root + 'Frag_Macacu_pontos.shp'

driver = ogr.GetDriverByName('ESRI Shapefile')

dsroad = driver.Open(roadfile,0)
dspoints = driver.Open(pointsfile,0)

def get_geometry(geom):
    g = []

    if geom.GetGeometryCount() > 0:
        i = 0
        while i < geom.GetGeometryCount():
            subgeom = geom.GetGeometryRef(i)
            g.append(subgeom.Clone())
            i = i + 1
    else:
        g.append(geom.Clone())

    return g        

def read_shape(f):
    geoms = []
    layer = f.GetLayer()
    feat = layer.GetNextFeature()
    while feat:
        geom = feat.GetGeometryRef()
        #print 'geom:',geom.GetGeometryName(),geom.GetGeometryCount(),geom.GetPointCount()
        print feat.DumpReadable()
        geoms.extend(get_geometry(geom))
        feat.Destroy()
        feat = layer.GetNextFeature()

    return geoms

def draw(g,isRoad):
    count = g.GetPointCount()
    i = 0
    if isRoad:
        while i < count-1:
            x1 = g.GetX(i)
            y1 = g.GetY(i)   
            x2 = g.GetX(i+1)
            y2 = g.GetY(i+1)
            plt.plot([x1,x2],[y1,y2],'-')
            #plt.plot([x1],[y1],'o')
            print x1,y1
            #i = i + 1
            i = i + 2
    else:            
        while i < count:
            x1 = g.GetX(i)
            y1 = g.GetY(i)   
            plt.plot([x1],[y1],'o')
            print x1,y1
            i = i + 1

points = read_shape(dspoints)
roads = read_shape(dsroad)

for p in points:
    draw(p,False)

for r in roads[1:3]:
    draw(r,True)

#for r in roads:
#    draw(r,True)

#print len(points)
#print len(roads)

#plt.plot([0,1,2,3],[1,6,13,22],'-')
#plt.plot([1,6,13,22],[0,1,2,3],'-')
plt.show()
