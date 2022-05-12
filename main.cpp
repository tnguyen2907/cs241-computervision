#include <iostream>
#include <fstream>
#include <string>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "main.h"

int baseline = 0;
int width = 0;
int height = 0;

using namespace std;
using namespace cv;

int main()
{
    string calib_path = "D:/Download/Project images/calib.txt";     //Path to calibration file
    ifstream calib_file(calib_path);

    string temp;
    while (getline(calib_file, temp)) {     // geting baseline, width, height
        //cout << temp << endl;
        if (temp.find("baseline=") != -1) baseline = stoi(temp.substr(9));
        else if (temp.find("width=") != -1) width = stoi(temp.substr(6));
        else if (temp.find("height=") != -1) height = stoi(temp.substr(7));
    }
    calib_file.close();

    cout << "Baseline: " << baseline << endl;
    cout << "Height: " << height << endl;
    cout << "Width: " << width << endl;

    string left_img_path = "D:/Download/Project images/im0.png";    //path to left image
    string right_img_path = "D:/Download/Project images/im1.png";   //path to right image

    //Image type: CV_8UC3 uchar (unsigned char)
    Mat left_img = imread(left_img_path);       //Left image
    Mat right_img = imread(right_img_path);     //Right image


    //Nam Anh
    Vec2b* key_pts_left = get_key_pts(left_img);        //malloc
    Vec2b* key_pts_right = get_key_pts(right_img);      //malloc
    //


    //Dung & Dat
    Vec2b* match_lst = match_key_pts(key_pts_left, key_pts_right, left_img, right_img);
    //


    free(match_lst);

    int** depth_map = (int**)malloc(height * sizeof(int*));     //Test depth_map
    int* temp_ptr;
    for (int i = 0; i < height; i++) {
        temp_ptr = (int*)malloc(width * sizeof(int));
        depth_map[i] = temp_ptr;
        for (int j = 0; j < width; j++) {
            temp_ptr[j] = j % 1024;
        }
    }

    print_depth_map(depth_map);


    for (int i = 0; i < height; i++) free(depth_map[i]);        //Free dynamic allocation
    free(depth_map);
    
    return 0;
}


