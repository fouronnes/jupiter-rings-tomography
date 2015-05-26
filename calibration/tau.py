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

def sort_split(stars_data):
    """
    Sort a star data matrix by 29x + y,
    such that the order is of increasing pixel value on the projector
    """

    y_num = 25
    x_step = 25
    y_step = 25
    weight = (y_num + 1) * y_step / x_step
    key = stars_data[:,0] * weight + stars_data[:,1]

    # Add key variable as the first column
    plused = append(key[:,None], stars_data, axis=1)

    # Sort and return the 3 original variables
    return np.array(sorted(plused, key=lambda x:x[0]))[:,1:]

def make_tau_plot(tau_range, all_images, noise_threshold):

    mti = 6 # minimum tau index
    tau_min = [0.05, 0.07, 0.035]

    for (m, (kind, images)) in enumerate(all_images.items()):
        f = figure(figsize=(8, 5))
        ax = f.add_subplot(111)

        ax.spines["top"].set_visible(False)
        ax.spines["right"].set_visible(False)
        ax.get_xaxis().tick_bottom()
        ax.get_yaxis().tick_left()

        tau = np.empty((25, len(images)))

        for (n, im) in enumerate(images):
            stars_data = extract_stars_data(im, noise_threshold)

            # Check we got 50 stars
            if stars_data.shape[0] != 50:
                print("WARNING: Extracted", stars_data.shape[0], "stars from image", n, "of", kind)

            star_data = sort_split(stars_data)

            I_zero = np.flipud(star_data[:25, 2])
            I_att = np.flipud(star_data[25:, 2])
            tau[:,n] = log(I_zero / I_att)

        tau_mean = np.mean(tau, axis=1)
        tau_std = np.std(tau, axis=1)

        ax.errorbar(tau_range[mti:], tau_mean[mti:], yerr=tau_std[mti:],
                linestyle='None', marker='.', color="black", lw=1, label=r"$log \frac{I_0}{I}$")
        ax.plot(tau_range[mti:], tau_range[mti:],
                '-', color="black", lw=1, label=r"$\tau$")

        ax.axvline(x=tau_min[m], linewidth=1, color='black', linestyle='--')
        ax.text(tau_min[m], 0.0015, r" $\tau_{min} = " + str(tau_min[m]) + r"$")

        ax.set_xscale('log', nonposy='clip')
        ax.set_yscale('log', nonposy='clip')

        ax.set_xlim([pow(10, -3.1), 1])
        ax.set_ylim([pow(10, -3.1), 1])
        
        i, st = kind[:-5], kind[-4:]
        ax.set_title(r"$I = " + str(i) + r"$  $\delta_t = " + str(st) + " \mu s$")
        ax.set_xlabel(r"Simulated optical depth")
        ax.set_ylabel(r"Measured optical depth")
        ax.legend(loc=2)
        f.show()
        f.savefig(str(i) + 'I' + str(st) + 'ST.pdf')

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

    make_tau_plot(tau_range, all_images, 0.17)

