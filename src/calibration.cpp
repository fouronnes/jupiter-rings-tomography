#include <iostream>
#include <cmath>
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"

typedef unsigned char uchar;

int main() {
    cv::namedWindow("Calibration", cv::WINDOW_AUTOSIZE);

    cv::Mat image = cv::Mat::zeros(600, 800, CV_32FC1);

    // Parameters
    int origin_x = 100;
    int origin_y = 100; 
    int x_step = 8;
    int y_step = 8;
    int max_x = 700;
    float p_step = 1.0 / (256*8);
    std::cout << p_step << std::endl;

    float p = 0.0;
    int x = 0;
    int y = 0;
    while (p <= 1.0) {
        image.at<float>(origin_y + y, origin_x + x) = p;
        x += x_step;
        p += p_step;
        if (origin_x + x > max_x) {
            x = 0;
            y += y_step;
        }
    }

    while(cvWaitKey(30) != 27) {
        cv::imshow("Calibration", image);
    }
}
