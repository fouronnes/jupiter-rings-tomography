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

        cv::Mat catline = (cv::Mat_<float>(1,3) << mag, ra, de);
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
            image.at<float>(y,x) += psf_gaussian(distance);
        }
    }
}

int main() {
    cv::namedWindow("Planetarium", cv::WINDOW_AUTOSIZE);

    cv::Mat image(600, 800, CV_32FC1);

    cv::Mat catalog = load_catalog("../hip5.tsv");

    draw_star(image, 100, 100.0);
    draw_star(image, 110, 100.1);
    draw_star(image, 120, 100.2);
    draw_star(image, 130, 100.3);
    draw_star(image, 140, 100.4);
    draw_star(image, 150, 100.4);
    draw_star(image, 160, 100.6);
    draw_star(image, 170, 100.7);
    draw_star(image, 180, 100.8);
    draw_star(image, 190, 100.9);
    draw_star(image, 200, 101.0);

    // Threshold to 1.0
    cv::threshold(image, image, 1.0, 0.0, cv::THRESH_TRUNC);

    while(cvWaitKey(5) != 27) {
        cv::imshow("Planetarium", image);
    }

    return 0;
}
