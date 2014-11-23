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
// The catalog is a (n,3) cv::Mat of mag, ra, de
cv::Mat load_catalog(const char* filename) {
    cv::Mat catalog(0, 3, CV_32FC1);
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

    void draw_star(cv::Mat image, float star_x, float star_y, float mag) const {
        // Intensity of the star we are rendering
        const float I_star = I_ref * pow(10, (m_ref - mag)/2.5);

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

    // True if a (ra,de) coordinate is visible on the screen
    bool is_star_visible(const double ra, const double de) const {
        const double max_ra = std::atan((screen_width * screen_horizontal_pixel_size) / (2*screen_distance));
        const double max_de = std::atan((screen_height * screen_vertical_pixel_size) / (2*screen_distance));

        // RA is in [0;2pi] but DE is in [-pi;pi]
        return (ra > 2*M_PI - max_ra || ra < max_ra)
            && (de > -max_de && de < max_de);
    }

    void draw_visible_stars(cv::Mat image, cv::Mat catalog) const {
        // For each entry in the catalog
        for (int i = 0; i < catalog.rows; i++) {
            float mag = catalog.at<float>(i, 0);
            double ra = catalog.at<float>(i, 1);
            double de = catalog.at<float>(i, 2);

            // Translate star by current attitude
            ra = angle_modulo(ra - attitude_ra, 0, 2*M_PI);
            de = angle_modulo(de - attitude_de, -M_PI, M_PI);

            // If star is visible
            if (is_star_visible(ra, de)) {
                const double x = screen_distance * tan(ra);
                const double y = screen_distance * tan(de);

                // Convert between:
                // Origin in the center, (x,y) in meters
                // Origin in the corner, (x_screen,y_screen) in pixels
                const int x_screen = round(-x / screen_horizontal_pixel_size + screen_width / 2);
                const int y_screen = round(-y / screen_vertical_pixel_size + screen_height / 2);

                draw_star(image, x_screen, y_screen, mag);
            }
        }
    }

    cv::Mat render(cv::Mat catalog) const {
        // Black background
        cv::Mat image = cv::Mat::zeros(screen_height, screen_width, CV_32FC1);

        draw_visible_stars(image, catalog);

        // Threshold to 1.0
        cv::threshold(image, image, 1.0, 0.0, cv::THRESH_TRUNC);

        return image;
    }
};

int main() {
    cv::namedWindow("Planetarium", cv::WINDOW_AUTOSIZE);

    planetarium wall;

    wall.psf_spread_size = 5;

    wall.screen_distance = .3;
    wall.screen_width = 1000;
    wall.screen_height = 700;
    wall.screen_horizontal_pixel_size = 0.0002 ;
    wall.screen_vertical_pixel_size = 0.0002;

    wall.attitude_ra = 45*M_PI/180;
    wall.attitude_de = 20*M_PI/180;

    wall.I_ref = 1.0;
    wall.m_ref = 3.0;
    wall.sigma_0 = 0.5;

    // Load star catalog
    cv::Mat catalog = load_catalog("../hip6.tsv");

    cv::Mat image = wall.render(catalog);

    int k;
    while((k = cvWaitKey(30)) != 27) {
        // Left
        if (k == 65361) {
            wall.attitude_ra += 1*M_PI/180;
            image = wall.render(catalog);
        }
        // Right
        else if (k == 65363) {
            wall.attitude_ra -= 1*M_PI/180;
            image = wall.render(catalog);
        }
        // Up
        else if (k == 65362) {
            wall.attitude_de += 1*M_PI/180;
            image = wall.render(catalog);
        }
        // Down
        else if (k == 65364) {
            wall.attitude_de -= 1*M_PI/180;
            image = wall.render(catalog);
        }

        cv::imshow("Planetarium", image);
    }

    return 0;
}
