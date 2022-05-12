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

Vec3b depth_to_color(int depth) {       //Map depth to a color in my custom spectrum
    return Vec3b(get_blue(depth), get_green(depth), get_red(depth));
}

int main()
{
    string calib_path = "D:/Download/Project images/calib.txt"; //Path to calibration file
    ifstream calib_file(calib_path);

    int width = 0;
    int height = 0;
    int baseline = 0;

    string temp;
    while (getline(calib_file, temp)) {  // geting baseline, width, height
        //cout << temp << endl;
        if (temp.find("baseline=") != -1) baseline = stoi(temp.substr(9));
        else if (temp.find("width=") != -1) width = stoi(temp.substr(6));
        else if (temp.find("height=") != -1) height = stoi(temp.substr(7));
    }
    calib_file.close();

    cout << "Baseline: " << baseline << endl;
    cout << "Height: " << height << endl;
    cout << "Width: " << width << endl;

    int** depth_map = (int**)malloc(height * sizeof(int*));     //Test depth_map
    int* temp_ptr;
    for (int i = 0; i < height; i++) {
        temp_ptr = (int*)malloc(width * sizeof(int));
        depth_map[i] = temp_ptr;
        for (int j = 0; j < width; j++) {
            temp_ptr[j] = j % 1024;
        }
    }


    Mat depth_map_img(height, width, CV_8UC3, Vec3b(0, 0, 0));  //Transform depth_map into a image with color representing depth
    for (int i = 0; i < depth_map_img.rows; i++) {
        for (int j = 0; j < depth_map_img.cols; j++) {
            depth_map_img.at<Vec3b>(i, j) = depth_to_color(depth_map[i][j]);
        }
    }

    for (int i = 0; i < height; i++) free(depth_map[i]);        //Free dynamic allocation
    free(depth_map);

    imshow("Depth map", depth_map_img);                         //Show depth map image
    waitKey(0);

    return 0;
}


