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

// Find the fundamental matrix F using the formula A.F = 0, 
// where A is a 9x9 matrix created by using the location of matched points from 2 images

// Create the matrix A in the equation from matched points
Mat lhs_mat(Point arr1[9], Point arr2[9]) {
    Mat lhs = Mat::zeros(9, 9, CV_64FC1);
    for (int i = 0; i < 9; i++) {
        lhs.at<double>(i, 0) += ((int)arr1[i].x) * ((int)arr2[i].x);
        lhs.at<double>(i, 1) += ((int)arr1[i].x) * ((int)arr2[i].y);
        lhs.at<double>(i, 2) += (int)arr1[i].x;
        lhs.at<double>(i, 3) += ((int)arr1[i].y) * ((int)arr2[i].x);
        lhs.at<double>(i, 4) += ((int)arr1[i].y) * ((int)arr2[i].y);
        lhs.at<double>(i, 5) += (int)arr1[i].y;
        lhs.at<double>(i, 6) += (int)arr2[i].x;
        lhs.at<double>(i, 7) += (int)arr2[i].y;
        lhs.at<double>(i, 8) += 1;
    }
    return lhs;
}

// Create the fundamental matrix using the equation and rearrange the entries
Mat fundamental(Point arr1[9], Point arr2[9]) {
    Mat lhs = lhs_mat(arr1, arr2);
    Mat sub_fund = Mat::zeros(9, 1, CV_64FC1);
    SVD::solveZ(lhs, sub_fund);             // solve for the 9x1 matrix which contains entries of the 3x3 fundamental matrix
    Mat fund = Mat::zeros(3, 3, CV_64FC1);  // rearrange entries of the fundamental matrix
    fund.at<double>(0, 0) += sub_fund.at<double>(0, 0);
    fund.at<double>(0, 1) += sub_fund.at<double>(1, 0);
    fund.at<double>(0, 2) += sub_fund.at<double>(2, 0);
    fund.at<double>(1, 0) += sub_fund.at<double>(3, 0);
    fund.at<double>(1, 1) += sub_fund.at<double>(4, 0);
    fund.at<double>(1, 2) += sub_fund.at<double>(5, 0);
    fund.at<double>(2, 0) += sub_fund.at<double>(6, 0);
    fund.at<double>(2, 1) += sub_fund.at<double>(7, 0);
    fund.at<double>(2, 2) += sub_fund.at<double>(8, 0);
    return fund;
}
