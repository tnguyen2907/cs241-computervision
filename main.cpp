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
int vmin = 0;       //Max disparity
int vmax = 0;       //Min disparity

using namespace std;
using namespace cv;

int main()
{
    folder_path = "D:/Download/Project images/";                  //Path to the folder that has the pair of images and the calibration file
    string calib_path = folder_path + "calib.txt";                  //Path to calibration file
    string left_img_path = folder_path + "im0.png";                 //path to left image
    string right_img_path = folder_path + "im1.png";                //path to right image
    string depth_map_path = folder_path + "depth_map.txt";          //Path to depth_map file


    ifstream calib_file(calib_path);

    string temp;
    while (getline(calib_file, temp)) {     // geting vmax, vmin
        if (temp.find("vmin=") != -1) vmin = stoi(temp.substr(5));
        else if (temp.find("vmax=") != -1) vmax = stoi(temp.substr(5));
    }
    calib_file.close();

    //Image type: CV_8UC3 uchar (unsigned char)
    Mat left_img = imread(left_img_path);       //Left image
    Mat right_img = imread(right_img_path);     //Right image

    width = left_img.cols;
    height = left_img.rows;

    cout << "Height: " << height << endl;
    cout << "Width: " << width << endl;

    ////////////Finding key points and match key points//////////
    /*
    Point left_pts[9];      //9 pairs
    Point right_pts[9];
   
    cout << "Finding key points\n";
    tuple<vector<KeyPoint>, vector<Mat>> left_tup = get_key_pts(left_img);
    tuple<vector<KeyPoint>, vector<Mat>> right_tup = get_key_pts(right_img);
        
    vector<KeyPoint> left_key_pts = get<0>(left_tup);
    vector<KeyPoint> right_key_pts = get<0>(right_tup);

    vector<Mat> left_descriptors = get<1>(left_tup);
    vector<Mat> right_descriptors = get<1>(right_tup);

    cout << "Finish finding key points\n";

    cout << "Start matching key points\n Will till about more than 3000\n";
    match_key_pts(left_descriptors, right_descriptors, left_key_pts, right_key_pts, left_pts, right_pts);
    cout << "Finish matching key points\n";
    
    cout << "Pairs of key points: \n";
    for (int i = 0; i < 9; i++) {
        cout << left_pts[i] << " ";
        cout << right_pts[i] << endl;
    }
    */
    //////////////////Find fundamental matrix///////////////////

    /*
    Mat fund = fundamental(left_pts, right_pts);
    cout << fund << endl;
    */

    ///////////////Rotate images///////////////////
    /*
    tuple<Mat, Mat> rectified_tup = rectify(left_img, right_img, fund);
    imshow("rectified left", get<0>(rectified_tup));
    waitKey(0);
    imshow("rectified right", get<1>(rectified_tup));
    waitKey(0);
    */

    ////////////////////////////////////////////////////////////

    int** depth_map = (int**)malloc(height * sizeof(int*));     //malloc
    int* temp_ptr;
    for (int i = 0; i < height; i++) {
        temp_ptr = (int*)malloc(width * sizeof(int));
        depth_map[i] = temp_ptr;
        for (int j = 0; j < width; j++) {
            temp_ptr[j] = 0;
        }
    }

    bool is_calculating = true;     //Set to true if calculate all over again

    if (is_calculating) {
        cout << "Start calculating depth map\n";

        calculate_depth_map(depth_map, 1, left_img, right_img);     //Calculate the depth map

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

    scale(depth_map);               //Normalize the depth map

    print_depth_map(depth_map);     //Output depth map


    for (int i = 0; i < height; i++) free(depth_map[i]);        //Free dynamic allocation
    free(depth_map);
    
    return 0;
}


