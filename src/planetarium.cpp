#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include <vector>
#include <algorithm>

#ifndef M_PI
    #define M_PI 3.14159265359
#endif

typedef unsigned char uchar;

// Load Hipparcos catalog
cv::Mat load_catalog(const char* filename) {
    cv::Mat catalog(1, 3, CV_32FC1);
    std::ifstream file(filename);
    std::string line, cell;

    if (!file) {
        std::cerr << "Can't open catalog " << filename << std::endl;
        return catalog;
    }

    // Skip first line
    std::getline(file, line);

    // Extract each line data
    while (std::getline(file, line)) {
        std::stringstream line_str(line);

        // Read fields
        double hip(0.0), mag(0.0), ra(0.0), de(0.0);
        line_str >> hip; line_str.get();
        line_str >> mag; line_str.get();
        line_str >> ra; line_str.get();
        line_str >> de; line_str.get();

        // Convert angles to radians
        const double deg_to_rad = M_PI / 180.0;
        cv::Mat catline = (cv::Mat_<float>(1,3) << mag, ra * deg_to_rad, de * deg_to_rad);
        catalog.push_back(catline);
    }
    return catalog;
}

struct planetarium {
    // Point spread function amplitude
    double psf_amplitude;

    // Point spread function b parameter
    double psf_sigma;

    // Max size of the spread to one side in pixels
    // e.g. 1 means the square of pixels modified will be 3x3
    double psf_spread_size;

    // Screen
    double screen_distance;
    double screen_width;
    double screen_height;
    double screen_horizontal_pixel_size;
    double screen_vertical_pixel_size;

    // The value of a gaussian point spread function at a given distance
    float psf_gaussian(float distance) {
        return psf_amplitude * exp(-(distance*distance) / psf_sigma);
    }

    // Draw a star at given screen coordinates
    // Uses a gaussian point spread function
    void draw_star(cv::Mat image, float star_x, float star_y) {
        // Closest discrete pixel to the star location
        const int center_x = round(star_x);
        const int center_y = round(star_y);

        // For each pixel around the star
        for (int x = center_x - psf_spread_size; x <= center_x + psf_spread_size; x++) {
            for (int y = center_y - psf_spread_size; y <= center_y + psf_spread_size; y++) {

                // Get the point spread function value at that distance from the star
                float distance = sqrt(pow(star_x - x, 2) + pow(star_y - y, 2));

                // If within bounds, add it to current value
                if (y > 0 && x > 0 && y < image.rows && x < image.cols) {
                    image.at<float>(y,x) += psf_gaussian(distance);
                }
            }
        }
    }

    // True if a (ra,de) coordinate is visible on the screen
    bool is_star_visible(const double ra, const double de) {
        const double max_ra = std::atan((screen_width * screen_horizontal_pixel_size) / (2*screen_distance));
        const double max_de = std::atan((screen_height * screen_vertical_pixel_size) / (2*screen_distance));

        // RA is in [0;2pi] but DE is in [-pi;pi]
        return ra - M_PI > -max_ra && ra - M_PI < max_ra && de > -max_de && de < max_de;
    }

    void draw_visible_stars(cv::Mat image, cv::Mat catalog) {
        // For each entry in the catalog
        for (int i = 0; i < catalog.rows; i++) {
            const double ra = catalog.at<float>(i, 1);
            const double de = catalog.at<float>(i, 2);

            // If star is visible
            if (is_star_visible(ra, de)) {
                const double x = screen_distance * tan(ra);
                const double y = screen_distance * tan(de);

                // Convert between:
                // Origin in the center, (x,y) in meters
                // Origin in the corner, (x_screen,y_screen) in pixels
                const int x_screen = round(-x / screen_horizontal_pixel_size + screen_width / 2);
                const int y_screen = round(-y / screen_vertical_pixel_size + screen_height / 2);

                draw_star(image, x_screen, y_screen);
            }
        }
    }
};

int main() {
    cv::namedWindow("Planetarium", cv::WINDOW_AUTOSIZE);

    // Black background
    cv::Mat image(600, 800, CV_32FC1);

    planetarium wall;

    wall.psf_amplitude = 1.0;
    wall.psf_sigma = 1.0;
    wall.psf_spread_size = 5;

    wall.screen_distance = .3;
    wall.screen_width = 800;
    wall.screen_height = 600;
    wall.screen_horizontal_pixel_size = 0.0002 ;
    wall.screen_vertical_pixel_size = 0.0002;

    // Load star catalog
    cv::Mat catalog = load_catalog("../hip5.tsv");

    wall.draw_visible_stars(image, catalog);

    // Threshold to 1.0
    cv::threshold(image, image, 1.0, 0.0, cv::THRESH_TRUNC);

    int k;
    while((k = cvWaitKey(5)) != 27) {
        cv::imshow("Planetarium", image);
        if (k != -1) {
            std::cout << k << std::endl;
        }
    }

    return 0;
}
