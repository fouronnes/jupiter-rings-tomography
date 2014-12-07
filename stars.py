import sys
import glob
import re
from pylab import *
from scipy.ndimage.measurements import center_of_mass
from collections import defaultdict

def find_next_while_pixel(image):
    """
    Coordinates of the first white pixel in a binary image
    Returns i,j image coordinates
    """
    coords = nonzero(image)
    if len(coords[0]) == 0:
        return None, None
    else:
        return coords[0][0], coords[1][0]

def construct_roi(im, i, j):
    """
    Discover the rectangular region of interest,
    starting from i,j
    """
    # Initialize ROI to 1x1
    w = 1
    h = 1
    
    # Extend the ROI to cover the whole star
    extending = True
    while extending:
        extending = False
        # If there a white pixel to the left of the ROI
        if im[i:i+h, j-1].any():
            extending = True
            j -= 1

        # Or to the right
        if im[i:i+h, j+w].any():
            extending = True
            w += 1

        # Or below
        if im[i+h, j:j+w].any():
            extending = True
            h += 1

    return i, j, w, h

def extract_stars(image, noise_threshold):
    """
    Extract all star from the given image
    Returns a list of rectangular images
    """

    roi_list = []
    image_list = []

    # Threshold to remove background noise
    image = image.copy()
    image[image < noise_threshold] = 0.0

    # Create binary image by thresholding
    binary = image.copy()
    binary[binary > 0] = 1

    # Find the next white pixel in the image
    i, j = find_next_while_pixel(binary)
    while i is not None and j is not None:

        # Construct the ROI around the pixel
        i, j, w, h = construct_roi(binary, i, j)

        # Save ROI to list or roi
        roi_list.append([i, j, w, h])

        # Erase ROI from image
        binary[i:i+h, j:j+w] = 0

        # Extract image region
        image_list.append(np.array(image[i:i+h, j:j+w]))

        # Find the next white pixel and repeat
        i, j = find_next_while_pixel(binary)

    return np.array(roi_list), image_list

def extract_stars_data(image, noise_threshold):
    """
    Input:  Calibration image with stars
    Output: List of stars data: x, y, intensity
    """

    roi_list, image_list = extract_stars(image, noise_threshold)

    stars_parameters = []
    for ((i,j,w,h), im) in zip(roi_list, image_list):
        cm_i, cm_j = center_of_mass(im)
        stars_parameters.append([j + cm_j, i + cm_i, im.sum()])

    return np.array(stars_parameters)

def get_gamma(st):
    key = st[:,1] * 29 + st[:,0]
    plused = append(key[:,None], st, axis=1)
    return sort(plused, axis=0)[:,3]

def make_calibration_curve(image, noise_threshold):

    star_data = extract_stars_data(image, noise_threshold)
    gamma = get_gamma(star_data)
    p_range = np.arange(0, 1, 1/(2*256))
    # assume non visible stars are too dark

    plot(p_range[-len(gamma):], gamma, '+', ms=1)
    show()

"""
Calibration images should follow the naming convention: <shutter time>ST<number>.bmp
For example: 20000ST1.bmp
"""
if __name__ == "__main__":
    folder = sys.argv[1]
    print("Loading calibration images from", folder)

    # Load all calibration images in a dictionary: st -> [np.array]
    all_images = defaultdict(list)

    for name in glob.glob(folder + "/*ST*.bmp"):

        # Extract path and shutter time
        pattern = folder + r"/([0-9]*)ST.*\.bmp"
        path, st = re.match(pattern, name).group(0, 1)

        # Load image and convert to float32
        im = imread(path)[:,:,0].astype(np.float32) / 255.0
        all_images[int(st)].append(im)

    total = sum([len(x) for x in all_images.values()])
    print("{} images loaded, shutter times found: {}".format(total, list(all_images.keys())))

