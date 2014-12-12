import sys
import glob
import re
from pylab import *
from collections import defaultdict

from stars import *

def load_tau_images(path):
    """
    Load all calibration images in a dictionary: kind -> [np.array]
    """
    all_images = defaultdict(list)

    for name in glob.glob(folder + "/*I*ST*.png"):

        # Extract path and shutter time
        pattern = folder + r"/([0-9]*I[0-9]*)ST.*\.png"
        path, kind = re.match(pattern, name).group(0, 1)

        # Load image and convert to float32
        # im = imread(path)[:,:].astype(np.float32) / 255.0
        im = imread(path)
        if im.ndim == 3:
            im = im[:,:,0]
        all_images[str(kind)].append(im)

    total = sum([len(x) for x in all_images.values()])
    print("{} images loaded, kinds found: {}".format(total, list(all_images.keys())))

    return all_images

"""
Tau images should follow the naming convention: <I_zero>I<shutter time>ST<number>.png
For example: 25I1000ST1.png
"""
if __name__ == "__main__":

    # Tau range
    tau_range = np.logspace(-4, -0.5, 25, endpoint=True, base=10.0)

    # Load tau images
    folder = sys.argv[1]
    print("Loading tau images from", folder)
    all_images = load_tau_images(folder)

    noise_threshold = 0.2

