#include <iostream>
#include <cmath>
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include <vector>
#include <algorithm>

typedef unsigned char uchar;

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
    std::cout << image.at<float>(100, 100) << std::endl;
    cv::threshold(image, image, 1.0, 0.0, cv::THRESH_TRUNC);
    std::cout << image.at<float>(100, 100) << std::endl;

    while(cvWaitKey(5) != 27) {
        cv::imshow("Planetarium", image);
    }

    return 0;
}
