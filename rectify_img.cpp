#define _USE_MATH_DEFINES

#include <stdio.h>
#include <cmath>
#include <string> 
#include <iostream>
#include <algorithm>
#include <tuple>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2/features2d.hpp>

using namespace cv;
using namespace std;

// Using the fundamental matrix and a point in the left image to find a line of the form ax + by + c = 0
// in the right image which contains the corresponding point to that point
Vec3f create_line_right(Mat fund, Point point) {
    float a = (((int)point.x) * fund.at<double>(0, 0)) + (((int)point.y) * fund.at<double>(1, 0)) + fund.at<double>(2, 0);
    float b = (((int)point.x) * fund.at<double>(0, 1)) + (((int)point.y) * fund.at<double>(1, 1)) + fund.at<double>(2, 1);
    float c = (((int)point.x) * fund.at<double>(0, 2)) + (((int)point.y) * fund.at<double>(1, 2)) + fund.at<double>(2, 2);
    return Vec3f(a, b, c);
}

// Using the fundamental matrix and a point in the right image to find a line of the form ax + by + c = 0
// in the left image which contains the corresponding point to that point
Vec3f create_line_left(Mat fund, Point point) {
    float a = (((int)point.x) * fund.at<double>(0, 0)) + (((int)point.y) * fund.at<double>(0, 1)) + fund.at<double>(0, 2);
    float b = (((int)point.x) * fund.at<double>(1, 0)) + (((int)point.y) * fund.at<double>(1, 1)) + fund.at<double>(1, 2);
    float c = (((int)point.x) * fund.at<double>(2, 0)) + (((int)point.y) * fund.at<double>(2, 1)) + fund.at<double>(2, 2);
    return Vec3f(a, b, c);
}

// Finding the angle between the line created by these 2 points and the horizontal line
float find_theta(Point center, Point spin) {
    return atan2f(((int)spin.y) - ((int)center.y), ((int)spin.x) - ((int)center.x));
}

// From the angle, create the rotation matrix which takes very point and rotate clockwise by an angle of theta 
Mat rot_mat(float theta) {
    Mat rot = Mat::zeros(2, 2, CV_32FC1);
    rot.at<float>(0, 0) += cos((2.0 * M_PI) - theta);
    rot.at<float>(0, 1) -= sin((2.0 * M_PI) - theta);
    rot.at<float>(1, 0) += sin((2.0 * M_PI) - theta);
    rot.at<float>(1, 1) += cos((2.0 * M_PI) - theta);
    return rot;
}

// Rotate every points in 2 images so that every pair of corresponding points in 2 images have the same y co-ordinate
tuple<Mat, Mat> rectify(Mat img1, Mat img2, Mat fund) {
    Point lpoint1((int)(img1.rows / 2), (int)(img1.cols / 2)); // pick the first point in the left image

    Vec3f right_line = create_line_right(fund, lpoint1);        // create the line in the right image
    int rp1_x = (int)(img2.rows / 2);
    int rp1_y = floor(-((right_line[2] + (right_line[0] * rp1_x)) / right_line[1]));
    Point rpoint1(rp1_x, rp1_y);                               // pick the first point in the right image 
    int rp2_x = (int)(img2.rows / 1.5);
    int rp2_y = floor(-((right_line[2] + (right_line[0] * rp2_x)) / right_line[1]));
    Point rpoint2(rp2_x, rp2_y);                                // pick the second point in the right image
    Mat right_rot = rot_mat(find_theta(rpoint1, rpoint2));      // find the rotation matrix for the right image

    Vec3f left_line = create_line_left(fund, rpoint1);          // create the line in the left image
    int lp2_y = rp1_y;
    int lp2_x = floor(-((left_line[2] + (left_line[1] * lp2_y)) / left_line[0]));
    Point lpoint2(lp2_x, lp2_y);                                // pick the second point in the left image
    int lp3_x = (int)(img1.rows / 2);
    int lp3_y = floor(-((left_line[2] + (left_line[0] * lp3_x)) / left_line[1]));
    Point lpoint3(lp3_x, lp3_y);                                // pick the third point in the left image
    Mat left_rot;                                               // create the rotation matrix for the left image
    if (lp3_x > lp2_x) left_rot = rot_mat(find_theta(lpoint2, lpoint3));
    else left_rot = rot_mat(find_theta(lpoint3, lpoint2));


    Mat new_img1(img1.rows, img1.cols, CV_8UC3, Vec3b(0, 0, 0)); // create a new left image with rotated points
    for (int i = 0; i < img1.rows; i++) {
        for (int j = 0; j < img1.cols; j++) {
            Mat tmp = (Mat_<float>(2, 1) << i, j);
            Mat loc = left_rot * tmp;
            if ((loc.at<float>(0, 0) >= 0) && ((int)loc.at<float>(0, 0) < img1.rows) && (loc.at<float>(1, 0) >= 0) && ((int)loc.at<float>(1, 0) < img1.cols)) {
                new_img1.at<Vec3b>((int)loc.at<float>(0, 0), (int)loc.at<float>(1, 0)) = img1.at<Vec3b>(i, j);
            }
        }
    }

    Mat new_img2(img2.rows, img2.cols, CV_8UC3, Vec3b(0, 0, 0));    // create a new right image with rotated points
    for (int i = 0; i < img2.rows; i++) {
        for (int j = 0; j < img2.cols; j++) {
            Mat tmp = (Mat_<float>(2, 1) << i, j);
            Mat loc = right_rot * tmp;

            if ((loc.at<float>(0, 0) >= 0) && ((int)loc.at<float>(0, 0) < img2.rows) && (loc.at<float>(1, 0) >= 0) && ((int)loc.at<float>(1, 0) < img2.cols)) {
                new_img2.at<Vec3b>((int)loc.at<float>(0, 0), (int)loc.at<float>(1, 0)) = img2.at<Vec3b>(i, j);
            }
        }
    }

    return make_tuple(new_img1, new_img2);
}