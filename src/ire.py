import math

class IRE:
    def __init__(self, A, B, K):
        self.A = A
        self.B = B
        self.K = K

    def forest_function(self,R):
        return (self.A*R + self.B)

    def h(self,R):
        t = 2*math.pi*R*self.forest_function(R)
        return (self.K/t)

       
