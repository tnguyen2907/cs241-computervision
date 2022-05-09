// input.cpp 
// Read from calib.txt to get baseline, width, height
// Input left and right images as left_img and right_img
// Image type: (probably) CV_8UC3 - uchar 8 bits (unsigned char), 3 color channels
// Note: Can use Vec3b to get a pixel and function at<...>(int row, int col)

#include <iostream>
#include <fstream>
#include <string>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;

int main()
{
    string calib_path = "D:/Download/Project images/calib.txt";     //Path to calibration file
    ifstream calib_file(calib_path);

    int width = 0;
    int height = 0;
    int baseline = 0;

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
    Mat left_img = imread(left_img_path);
    //imshow("Left Image", left_img);       // Show image
    //waitKey(0);
    Mat right_img = imread(right_img_path);
    /*  imshow("Right Image", right_img);
      waitKey(0);*/

      //cout << "R (numpy)   = " << endl << format(left_img, Formatter::FMT_NUMPY) << endl << endl;   //Print whole array (very big)
      //cout << left_img.at<Vec3b>(100, 200) << endl;     //Print a pixel
    return 0;
}


