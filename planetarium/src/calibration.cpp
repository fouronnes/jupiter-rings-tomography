#include <iostream>
#include <cmath>
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"

typedef unsigned char uchar;

int main() {
    cv::namedWindow("Calibration", cv::WINDOW_NORMAL);

    cv::Mat image = cv::Mat::zeros(1080, 1920, CV_32FC1);

    // Parameters
    int origin_x = 400;
    int origin_y = 300; 
    int x_step = 40;
    int y_step = 40;
    int max_x = 1520;
    float p_step = 1.0 / (256*2);
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
        std::cout << p << std::endl;
    }

    cv::flip(image, image, 0);

    int k;
    while((k = cvWaitKey(30)) != 27) {
        cv::imshow("Calibration", image);

        if (k == 102) {
            cvSetWindowProperty("Calibration", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN );
        }
    }

    cv::Mat image_uchar;
    image.convertTo(image_uchar, CV_8UC1, 255);
    cv::imwrite("calibration.png", image_uchar);
}
