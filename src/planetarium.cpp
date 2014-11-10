#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include <vector>
#include <algorithm>

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
        const double deg_to_rad = 3.14159265359 / 180.0;
        cv::Mat catline = (cv::Mat_<float>(1,3) << mag, ra * deg_to_rad, de * deg_to_rad);
        catalog.push_back(catline);
    }
    return catalog;
}

// The value of a gaussian point spread function at a given distance
float psf_gaussian(float distance) {
    // Amplitude and std
    return 1.0 * exp(-(distance*distance) / 1.0);
}

// Draw a star at given screen coordinates
// Uses a gaussian point spread function
void draw_star(cv::Mat image, float star_x, float star_y) {

    // Max size of the spread to one side in pixels
    // e.g. 1 means the square of pixels modified will be 3x3
    const int spread_size = 5;

    // Closest discrete pixel to the star location
    const int center_x = round(star_x);
    const int center_y = round(star_y);
    
    // For each pixel around the star
    for (int x = center_x - spread_size; x <= center_x + spread_size; x++) {
        for (int y = center_y - spread_size; y <= center_y + spread_size; y++) {

            // Get the point spread function value at that distance from the star
            float distance = sqrt(pow(star_x - x, 2) + pow(star_y - y, 2));

            // If within bounds, add it to current value
            if (y > 0 && x > 0 && y < image.rows && x < image.cols) {
                image.at<float>(y,x) += psf_gaussian(distance);
            }
        }
    }
}

void draw_visible_stars(cv::Mat image, cv::Mat catalog) {
    const double screen_distance = 1.0;
    const double screen_width = 800;
    const double screen_height = 600;
    const double pixel_size = 0.002;

    for (int i = 0; i < catalog.rows; i++) {
        double ra = catalog.at<float>(i, 1);
        double de = catalog.at<float>(i, 2);

        if (ra > -1.5 && ra < 1.5 && de > -1.5 && de < 1.5) {
            double x = screen_distance * tan(ra);
            double y = screen_distance * tan(de);

            // Meters to pixel
            int x_screen = round(-x / pixel_size + screen_width / 2);
            int y_screen = round(-y / pixel_size + screen_height / 2);

            std::cout << x_screen << " " << y_screen  << std::endl;

            if (x_screen > 0 && x_screen < screen_width && y_screen > 0 && y_screen < screen_height) {
                draw_star(image, x_screen, y_screen);
            }
        }
    }
}

int main() {
    cv::namedWindow("Planetarium", cv::WINDOW_AUTOSIZE);

    // Black background
    cv::Mat image(600, 800, CV_32FC1);

    // Load star catalog
    cv::Mat catalog = load_catalog("../hip5.tsv");

    draw_visible_stars(image, catalog);

    // Threshold to 1.0
    cv::threshold(image, image, 1.0, 0.0, cv::THRESH_TRUNC);

    while(cvWaitKey(5) != 27) {
        cv::imshow("Planetarium", image);
    }

    return 0;
}
