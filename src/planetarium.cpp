#include <iostream>
#include <cmath>
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include <vector>
#include <algorithm>

typedef unsigned char uchar;

int main() {
    cv::namedWindow("Planetarium", cv::WINDOW_AUTOSIZE);

    while(cvWaitKey(5) != 27) {

    }

    return 0;
}
