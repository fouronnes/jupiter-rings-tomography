from pylab import *
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.path import Path
import matplotlib.patches as patches

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

def draw_jupiter():
    # autoscale(False)
    c = Circle((0, 0), 69911, fill=True, color="#c18863")   
    gca().add_artist(c)
    # autoscale(True)

def draw_ring():
    minx = 92000
    maxx = 226000
    hw = 10000 # halfwidth

    verts = [
        (minx, -hw), # left, bottom
        (minx, hw), # left, top
        (maxx, hw), # right, top
        (maxx, -hw), # right, bottom
        (0., 0.), # ignored
        ]

    codes = [Path.MOVETO, Path.LINETO, Path.LINETO, Path.LINETO, Path.CLOSEPOLY]

    path = Path(verts, codes)

    fig = gcf()
    ax = gca()
    patch = patches.PathPatch(path, facecolor='gray', lw=0)
    ax.add_patch(patch)

def test():

    RA = attitude[0,0]
    DE = attitude[0,1]

    f = figure()
    ax = f.add_subplot(111, aspect='equal')
    s = project_state(juno[:, [1,2,3]])
    plot(s[:,0], s[:,1])
    draw_jupiter()

    perigee_time = 5.3112e8
    max_date = perigee_time + 3500
    min_date = perigee_time - 400
    relevant = juno[juno[:,0] < max_date,:]
    relevant = relevant[relevant[:,0] > min_date,:]

    for state in relevant:
        x,y = make_ray(state[1], state[2], state[3], RA, DE, 500000)
        plot(x,y, '-', linewidth=1, color="black")

    draw_ring()
    show()

test()

