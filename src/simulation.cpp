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

// Equivalent angle in specified interval
double angle_modulo(double angle, double min, double max) {
    while (angle < min) {
        angle += 2 * M_PI;
    }
    while (angle > max) {
        angle -= 2 * M_PI;
    }
    return angle;
}

struct planetarium {
    // Max size of the spread to one side in pixels
    // e.g. 1 means the square of pixels modified will be 3x3
    double psf_spread_size;

    // Screen
    double screen_distance; // Meters
    double screen_width; // Pixels
    double screen_height; // Pixels
    double screen_horizontal_pixel_size; // Meters per pixel
    double screen_vertical_pixel_size; // Meters per pixel

    // Attitude in radians
    double attitude_ra;
    double attitude_de;

    // Star rendering parameters
    double I_ref, m_ref, sigma_0;

    // Camera position with respect to screen center
    double camera_x_offset, camera_y_offset; // Meters

    // Linear inverse gamma function
    float gamma_inverse(float intensity) const {
        float gamma = I_ref;
        return intensity / gamma;
    }

    // The gaussian function
    float psf_gaussian(float distance, float alpha, float sigma) const {
        return alpha * exp(-(distance*distance) / (sigma*sigma));
    }

    // Draw a gaussian psf at given screen coordinates
    void draw_gaussian(cv::Mat image, float star_x, float star_y, float alpha, float sigma) const {
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
                    image.at<float>(y,x) += gamma_inverse(psf_gaussian(distance, alpha, sigma));
                }
            }
        }
    }

    void draw_star(cv::Mat image, float star_x, float star_y, float I_star) const {
        // Maximum for rendering star with fixed sigma
        const float I_max = I_ref * sigma_0 * sigma_0 * M_PI;

        float alpha, sigma;
        if (I_star <= I_max) {
            alpha = I_star / (sigma_0 * sigma_0 * M_PI);
            sigma = sigma_0;
        }
        else {
            alpha = I_ref;
            sigma = sqrt(I_star / (I_ref * M_PI));
        }

        draw_gaussian(image, star_x, star_y, alpha, sigma);
    }
};

int main() {
    cv::namedWindow("Simulation", cv::WINDOW_NORMAL);

    planetarium wall;

    wall.psf_spread_size = 8;

    wall.screen_distance = 5.954;
    wall.screen_width = 1920;
    wall.screen_height = 1080;
    wall.screen_horizontal_pixel_size = 2.364 / wall.screen_width;
    wall.screen_vertical_pixel_size = 1.335 / wall.screen_height;

    wall.attitude_ra = 0;
    wall.attitude_de = 0;

    wall.I_ref = 1.0;
    wall.m_ref = 6.0;
    wall.sigma_0 = 1.0;

    wall.camera_x_offset = 0.0;
    wall.camera_y_offset = 0.0;

    // Black background
    cv::Mat image = cv::Mat::zeros(wall.screen_height, wall.screen_width, CV_32FC1);

    // Reference intensity for the simulation
    const float I_zero = 1.0 * pow(wall.sigma_0, 2) * M_PI;
    const float x_step = 40;
    const float y_step = 40;
    
    // np.logspace(-4, -1, 40, endpoint=True, base=10.0)
    std::vector<float> taus =
        { 0.0001    ,  0.00011938,  0.00014251,  0.00017013,  0.00020309,
        0.00024245,  0.00028943,  0.00034551,  0.00041246,  0.00049239,
        0.0005878 ,  0.0007017 ,  0.00083768,  0.001     ,  0.00119378,
        0.0014251 ,  0.00170125,  0.00203092,  0.00242446,  0.00289427,
        0.00345511,  0.00412463,  0.00492388,  0.00587802,  0.00701704,
        0.00837678,  0.01      ,  0.01193777,  0.01425103,  0.01701254,
        0.02030918,  0.02424462,  0.02894266,  0.03455107,  0.04124626,
        0.04923883,  0.05878016,  0.07017038,  0.08376776,  0.1 };

    float x = 100;
    float y = wall.screen_height/2 - y_step/2;

    for (auto tau : taus) {
        x += x_step;

        // Draw reference star
        wall.draw_star(image, x, y, I_zero);
        
        // Draw attenuated star below
        float I = I_zero * exp(-tau);
        wall.draw_star(image, x, y + y_step, I);
    }

    int k;
    while((k = cvWaitKey(30)) != 27) {
        if (k == 102) {
            cvSetWindowProperty("Simulation", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN );
        }
        cv::imshow("Simulation", image);
    }

    // cv::Mat image_uchar;
    // image.convertTo(image_uchar, CV_8UC1, 255);
    // cv::imwrite("simulation.png", image_uchar);

    return 0;
}
