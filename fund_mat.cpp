#define _USE_MATH_DEFINES

#include <stdio.h>
#include <cmath>
#include <string> 
#include <iostream>
#include <algorithm>
#include <tuple>
#include <opencv2\opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2/features2d.hpp>

using namespace cv;
using namespace std;

Mat lhs_mat(Point arr1[9], Point arr2[9]) {
    Mat lhs = Mat::zeros(9, 9, CV_64FC1);
    for (int i = 0; i < 9; i++) {
        lhs.at<float>(i, 0) += ((int) arr1[i].x) * ((int) arr2[i].x);
        lhs.at<float>(i, 1) += ((int) arr1[i].x) * ((int) arr2[i].y);
        lhs.at<float>(i, 2) += (int) arr1[i].x;
        lhs.at<float>(i, 3) += ((int) arr1[i].y) * ((int) arr2[i].x);
        lhs.at<float>(i, 4) += ((int) arr1[i].y) * ((int) arr2[i].y);
        lhs.at<float>(i, 5) += (int) arr1[i].y;
        lhs.at<float>(i, 6) += (int) arr2[i].x;
        lhs.at<float>(i, 7) += (int) arr2[i].y;
        lhs.at<float>(i, 8) += 1;
    }
    return lhs;
}


Mat fundamental(Mat lhs) {
    Mat rhs = Mat::zeros(9 , 1, CV_64FC1);
    //rhs.at<float>(8,0) += 9;
    Mat sub_fund = Mat::zeros(9 , 1, CV_64FC1);
    // Mat inv_mat;
    // invert(lhs, inv_mat)
    SVD::solveZ(lhs, sub_fund);

    Mat fund = Mat::zeros(3 , 3, CV_64FC1);
    fund.at<float>(0,0) += sub_fund.at<float>(0,0);
    fund.at<float>(0,1) += sub_fund.at<float>(1,0);
    fund.at<float>(0,2) += sub_fund.at<float>(2,0);
    fund.at<float>(1,0) += sub_fund.at<float>(3,0);
    fund.at<float>(1,1) += sub_fund.at<float>(4,0);
    fund.at<float>(1,2) += sub_fund.at<float>(5,0);
    fund.at<float>(2,0) += sub_fund.at<float>(6,0);
    fund.at<float>(2,1) += sub_fund.at<float>(7,0);
    fund.at<float>(2,2) += sub_fund.at<float>(8,0);
    return fund;
}


