import scipy.integrate as integrate 
import numpy as np

from scipy import linalg

class ForestIndex:
    def __init__(self, index_function, roads_list, centers):
        self.index_function = index_function
        self.roads_list = roads_list
        self.centers = centers

    def param_line(self,t,start,end):
        return np.array([start[0] + t*(end[0]-start[0]),start[1] + t*(end[1]-start[1])])

    def quad_linesegment(self,c,p1,p2):
        return integrate.quad(lambda t:
                linalg.norm(np.array([p1[0]-p2[0],p1[1]-p2[1]]))*self.index_function.h(linalg.norm(c-self.param_line(t,p1,p2))),0,1)

    def line_integral(self,roads,point):
        error = 0
        total = 0
        length = 0
        for road in roads:
            length = length + self.road_length(road)
            p1 = road[0]        
            for p in road[1:]:
                (y,abserr) = self.quad_linesegment(point,p1,p)
                p1 = p
                total = total + y
                error = error + abserr

        avire = total/length
        return (total,avire,error)        

    def road_length(self,r):
        p1 = r[0]
        length = 0
        for p in r[1:]:
            length = length + linalg.norm(p-p1)
            p1 = p
        return length

    def calculate_index(self):
        idx = []

        for i in range(0,(len(self.centers)-1)):
            roads = self.roads_list[i]
            if len(roads) > 0:
                (ti,ati,ei) = self.line_integral(roads,self.centers[i])
                idx.append([i,ti,ati,ei])

        return idx

