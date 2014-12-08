from pylab import *
from mpl_toolkits.mplot3d import Axes3D

X = 100
Y = 0
Z = 0

ax = 0.1
ay = 1
az = 0.2

y = linspace(-100, 100, 1000)

x = (X + (y - Z)*ax/az)**2 + (Y + (y - Z)*ay/az)**2

def plot3d(x, y, z):
    f = figure()
    ax = f.add_subplot(111, projection='3d')
    ax.plot(x, y, z, '.')
    show()
