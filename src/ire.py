from osgeo import ogr
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.lines as lines

def draw_points(g):
    count = g.GetPointCount()
    i = 0
    while i < count-1:
        x1 = g.GetX(i)
        y1 = g.GetY(i)   
        x2 = g.GetX(i+1)
        y2 = g.GetY(i+1)
        #plt.plot([x1,x2],[y1,y2],'-')
        i = i + 2

root = './data/gis/'
driver = ogr.GetDriverByName('ESRI Shapefile')

ds = driver.Open(root+'florestotal.shp',0)

layer = ds.GetLayer()
print layer.GetFeatureCount()
print ds.GetLayerCount()

feat = layer.GetNextFeature()
while feat:
    geom = feat.GetGeometryRef()
    #print 'geom:',geom.GetGeometryName(),geom.GetGeometryCount(),geom.GetPointCount()
    if geom.GetGeometryCount() > 0:
        i = 0
        while i < geom.GetGeometryCount():
            subgeom = geom.GetGeometryRef(i)
            #print 'subgeom:',i, subgeom.GetGeometryName(),subgeom.GetPointCount()
            draw_points(subgeom)
            i = i + 1
    else:
        draw_points(geom)
    print feat.DumpReadable()
    feat.Destroy()
    feat = layer.GetNextFeature()

#plt.plot([0,1,2,3],[1,6,13,22],'-')
#plt.plot([1,6,13,22],[0,1,2,3],'-')
plt.show()
