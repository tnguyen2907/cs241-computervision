// output.cpp 
// Input: need an 2d array of int (int ** depth_map). Each element in the array is the depth of the point at that location
//        the range of depth_map should be 0 -> 767 
// Output: an image representing the depth map
// 
// My custom spectrum
//   /\  /\  /\    Max: 255; Range: 0 -> 1023
//  /  \/  \/  \
// /   /\  /\   \
// Blue Green Red  

#include <iostream>
#include <fstream>
#include <string>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
#include "main.h"

using namespace std;
using namespace cv;


int get_blue(int n) {
    if (n < 512) {
        return (-1 * n * (n - 511)) / 256;
    }
    else return 0;
}

int get_green(int n) {
    if (n >= 256 && n < 768) {
        return (-1 * (n - 256) * (n - 767)) / 256;
    }
    else return 0;
}

int get_red(int n) {
    if (n >= 512 && n < 1024) {
        return (-1 * (n - 512) * (n - 1023)) / 256;
    }
    else return 0;
}

Vec3b depth_to_color(int depth) {       //Map depth to a color in spectrum
    return Vec3b(get_blue(depth), get_green(depth), get_red(depth));
}

void print_depth_map(int** depth_map){

    Mat depth_map_img(height, width, CV_8UC3, Vec3b(0, 0, 0));  //Transform depth_map into a image with color representing depth
    for (int i = 0; i < depth_map_img.rows; i++) {
        for (int j = 0; j < depth_map_img.cols; j++) {
            depth_map_img.at<Vec3b>(i, j) = depth_to_color(depth_map[i][j]);
        }
    }
    //If want to save depth map, uncomment this
    //imwrite(folder_path + "optimized_map.png", depth_map_img);

    Mat resized_img;
    resize(depth_map_img, resized_img, Size(0.7 * width, 0.7 * height));    //resize to fit 

    imshow("Depth map", resized_img);                         //Show depth map image
    waitKey(0);

}


