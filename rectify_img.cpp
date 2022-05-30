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

Vec3f create_line_right(Mat &fund, Point &point) {
    float a = (((int) point.x) * fund.at<float>(0, 0)) + (((int) point.y) * fund.at<float>(1, 0)) + fund.at<float>(2, 0);
    float b = (((int) point.x) * fund.at<float>(0, 1)) + (((int) point.y) * fund.at<float>(1, 1)) + fund.at<float>(2, 1);
    float c = (((int) point.x) * fund.at<float>(0, 2)) + (((int) point.y) * fund.at<float>(1, 2)) + fund.at<float>(2, 2);
    return Vec3f(a, b, c);
}

Vec3f create_line_left(Mat &fund, Point &point){
    float a = (((int) point.x) * fund.at<float>(0, 0)) + (((int) point.y) * fund.at<float>(0, 1)) + fund.at<float>(0, 2);
    float b = (((int) point.x) * fund.at<float>(1, 0)) + (((int) point.y) * fund.at<float>(1, 1)) + fund.at<float>(1, 2);
    float c = (((int) point.x) * fund.at<float>(2, 0)) + (((int) point.y) * fund.at<float>(2, 1)) + fund.at<float>(2, 2);
    return Vec3f(a, b, c);
}

float find_theta(Point &center, Point &spin){
    return atan2f(((int) spin.y) - ((int) center.y), ((int) spin.x) - ((int) center.x));
}

Mat rot_mat(float theta){
    Mat rot = Mat::zeros(2, 2, CV_32FC1);
    rot.at<float>(0, 0) += cos((2.0 * M_PI) - theta);
    rot.at<float>(0, 1) -= sin((2.0 * M_PI) - theta);
    rot.at<float>(1, 0) += sin((2.0 * M_PI) - theta);      
    rot.at<float>(1, 1) += cos((2.0 * M_PI) - theta); 
    return rot;
}

tuple<Mat, Mat> rectify(Mat &img1, Mat &img2, Mat &fund){
    Point lpoint1((int) (img1.rows / 2), (int) (img1.cols / 2));

    Vec3f right_line = create_line_right(fund, lpoint1);
    int rp1_x = (int) (img2.rows /2);
    int rp1_y = floor(- ((right_line[2] + (right_line[0] * rp1_x)) / right_line[1]));
    Point rpoint1(rp1_x, rp1_y);
    int rp2_x = (int) (img2.rows /1.5);
    int rp2_y = floor(- ((right_line[2] + (right_line[0] * rp2_x)) / right_line[1]));
    Point rpoint2(rp2_x, rp2_y);
    Mat right_rot = rot_mat(find_theta(rpoint1, rpoint2));

    Vec3f left_line = create_line_left(fund, rpoint1);
    int lp2_y = rp1_y;
    int lp2_x = floor(- ((left_line[2] + (left_line[1] * lp2_y)) / left_line[0]));
    Point lpoint2(lp2_x, lp2_y);
    int lp3_x = (int) (img1.rows / 2);
    int lp3_y = floor(- ((left_line[2] + (left_line[0] * lp3_x)) / left_line[1]));
    Point lpoint3(lp3_x, lp3_y);
    Mat left_rot;
    if (lp3_x > lp2_x) left_rot = rot_mat(find_theta(lpoint2, lpoint3));
    else left_rot = rot_mat(find_theta(lpoint3, lpoint2));

    Mat new_img1(img1.rows, img1.cols, CV_8UC3, Vec3b(0, 0, 0));
    for (int i = 0; i < img1.rows; i++){
        for (int j = 0; j < img1.cols; j++){
            Mat tmp = (Mat_<float>(2, 1) << i, j);
            Mat loc = left_rot * tmp;
            if ((loc.at<float>(0, 0) >= 0) && ((int) loc.at<float>(0, 0) < img1.rows) && (loc.at<float>(1, 0) >= 0) && ((int) loc.at<float>(1,0) < img1.cols)) {
                new_img1.at<Vec3b>((int) loc.at<float>(0,0) , (int) loc.at<float>(1,0)) = img1.at<Vec3b>(i, j);
            }
        }
    }
    
    Mat new_img2(img2.rows, img2.cols, CV_8UC3, Vec3b(0, 0, 0));
    for (int i = 0; i < img2.rows; i++){
        for (int j = 0; j < img2.cols; j++){
            Mat tmp = (Mat_<float>(2, 1) << i, j);
            Mat loc = right_rot * tmp;
            
            if ((loc.at<float>(0, 0) >= 0) && ((int) loc.at<float>(0, 0) < img2.rows) && (loc.at<float>(1, 0) >= 0) && ((int) loc.at<float>(1,0) < img2.cols)) {
                new_img2.at<Vec3b>((int) loc.at<float>(0,0) , (int) loc.at<float>(1,0)) = img2.at<Vec3b>(i, j);
            }
        }
    }

    return make_tuple(new_img1, new_img2);
}

int main(int argc, char **argv)
{
    Mat image_left = imread("left.bmp");
    Mat image_right = imread("right.bmp");
    Mat fund = (Mat_<float>(3, 3) << -.003, -.028, 13.19, -.003, -.008, -29.2, 2.97, 50, -9999);
    tuple<Mat, Mat> ret = rectify(image_left, image_right, fund);
    imshow("left", get<0>(ret));
    waitKey(0);
    imshow("right", get<1>(ret));
    waitKey(0);
}