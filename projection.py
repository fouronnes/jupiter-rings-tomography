from pylab import *
from mpl_toolkits.mplot3d import Axes3D

# Load juno orbit 1
juno = np.loadtxt('juno-state-orbit1.txt')

# Load attitude file
attitude = np.loadtxt('juno-attitude.txt')

def project_state(state):
    """
    Project 3D state to the vertical ring plane
    using cylindrical coordinates.
    Input: n,3 matrix (X,Y,Z)
    Output: n,2 matrix (X,Y)
    """
    x = sqrt(state[:,0]**2 + state[:,1]**2)
    y = state[:,2]
    return np.vstack((x,y)).T

def make_ray(X, Y, Z, RA, DE, length):
    ax = cos(DE) * cos(RA)
    ay = cos(DE) * sin(RA)
    az = sin(DE)

    mu = linspace(0, length, length/1000)
    x = sqrt((X + mu*ax)**2 + (Y + mu*ay)**2)
    y = Z + mu*az

    return x,y

def plot3d(x, y, z):
    f = figure()
    ax = f.add_subplot(111, projection='3d')
    ax.plot(x, y, z, '.')
    show()
