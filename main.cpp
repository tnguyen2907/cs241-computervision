#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include "main.h"

std::string folder_path;
int width = 0;
int height = 0;
int vmin = 0;
int vmax = 0;

using namespace std;
using namespace cv;

int main()
{
    folder_path = "D:/Download/Project images 3/";
    string calib_path = folder_path + "calib.txt";     //Path to calibration file
    ifstream calib_file(calib_path);

    string temp;
    while (getline(calib_file, temp)) {     // geting baseline, width, height
        //cout << temp << endl;
        if (temp.find("vmin=") != -1) vmin = stoi(temp.substr(5));
        else if (temp.find("vmax=") != -1) vmax = stoi(temp.substr(5));
    }
    calib_file.close();

    string left_img_path = folder_path + "im0.png";    //path to left image
    string right_img_path = folder_path + "im1.png";   //path to right image

    //Image type: CV_8UC3 uchar (unsigned char)
    Mat left_img = imread(left_img_path);       //Left image
    Mat right_img = imread(right_img_path);     //Right image

    width = left_img.cols;
    height = left_img.rows;

    cout << "Height: " << height << endl;
    cout << "Width: " << width << endl;

    Point left_pts[9];
    Point right_pts[9];
   
    string pair_path = folder_path + "pair.txt";     //Path to matching pair file

    bool is_get_pair = true;            //Set to true if want to find key points all over again
    if (is_get_pair) {
        cout << "Finding key points\n";
        tuple<vector<KeyPoint>, vector<Mat>> left_tup = get_key_pts(left_img);
        tuple<vector<KeyPoint>, vector<Mat>> right_tup = get_key_pts(right_img);

        vector<KeyPoint> left_key_pts = get<0>(left_tup);
        vector<KeyPoint> right_key_pts = get<0>(right_tup);

        vector<Mat> left_descriptors = get<1>(left_tup);
        cout << format(left_descriptors[0], Formatter::FMT_NUMPY) << endl << endl;
        vector<Mat> right_descriptors = get<1>(right_tup);

        cout << "Finish finding key points\n";

        match_key_pts(left_descriptors, right_descriptors, left_key_pts, right_key_pts, left_pts, right_pts);

        ofstream pair_file(pair_path);

        for (int i = 0; i < 9; i++) {
            pair_file << (int)left_pts[i].x << " ";
            pair_file << (int)left_pts[i].y << " ";
        }
        pair_file << "\n";

        for (int i = 0; i < 9; i++) {
            pair_file << (int)right_pts[i].x << " ";
            pair_file << (int)right_pts[i].y << " ";
        }

        pair_file.close();
    }
    else {
        ifstream pair_file(pair_path);
        int x = 0;
        int y = 0;
        for (int i = 0; i < 9; i++) {
            pair_file >> x;
            pair_file >> y;
            left_pts[i] = Point(x, y);
        }

        for (int i = 0; i < 9; i++) {
            pair_file >> x;
            pair_file >> y;
            right_pts[i] = Point(x, y);
        }

        pair_file.close();
    }

    Mat fund = fundamental(left_pts, right_pts);
    cout << fund << endl;

    tuple<Mat, Mat> rectified_tup = rectify(left_img, right_img, fund);
    imshow("rectified left", get<0>(rectified_tup));
    waitKey(0);
    imshow("rectified right", get<1>(rectified_tup));
    waitKey(0);

    int** depth_map = (int**)malloc(height * sizeof(int*));     
    int* temp_ptr;
    for (int i = 0; i < height; i++) {
        temp_ptr = (int*)malloc(width * sizeof(int));
        depth_map[i] = temp_ptr;
        for (int j = 0; j < width; j++) {
            temp_ptr[j] = 0;
        }
    }

    string depth_map_path = folder_path + "depth_map.txt";     //Path to depth_map file

    bool is_calculating = true;     //Set to true if calculate all over again

    if (is_calculating) {
        cout << "Start calculating depth map\n";

        calculate_depth_map(depth_map, 2, left_img, right_img);

        cout << "Finish calculate depth map\n";

        ofstream depth_map_file(depth_map_path);

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                depth_map_file << depth_map[i][j] << " ";
            }
            depth_map_file << "\n";
        }
        depth_map_file.close();
    }
    else {
        ifstream depth_map_file(depth_map_path);

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                depth_map_file >> depth_map[i][j];
            }
        }
        depth_map_file.close();
    }

    scale(depth_map);

    print_depth_map(depth_map);


    for (int i = 0; i < height; i++) free(depth_map[i]);        //Free dynamic allocation
    free(depth_map);
    
    return 0;
}


