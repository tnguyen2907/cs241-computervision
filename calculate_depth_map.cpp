#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "main.h"
#include <cmath>

using namespace std;
using namespace cv;

double distance(Vec3b p1, Vec3b p2) {
	return sqrt(pow((int)p1[0] - (int)p2[0], 2) + pow((int)p1[1] - (int)p2[1], 2) + pow((int)p1[2] - (int)p2[2], 2));
}

double ssd(int window_size, Mat m1, Mat m2) {		//Sum of square difference
	double sum = 0;
	for (int i = 0; i < window_size; i++) {
		for (int j = 0; j < window_size; j++) {
			sum += pow(distance(m1.at<Vec3b>(i, j), m2.at<Vec3b>(i, j)), 2);
		}
	}
	return sum;
}

int match_left_with_right(int window_size, int left_x_pos, int y_pos, Mat left_img, Mat right_img) {
	Mat left = left_img.colRange(left_x_pos, left_x_pos + window_size).rowRange(y_pos, y_pos + window_size);
	int right_x_pos = 0;
	Mat cur_right = right_img.colRange(right_x_pos, right_x_pos + window_size).rowRange(y_pos, y_pos + window_size);

	int start = left_x_pos - vmax;
	if (start < 0) start = 0;
	int end = left_x_pos;
	//if (end > width - window_size) end = width - window_size;

	for (int i = start; i < end; i += window_size) {
		Mat temp = right_img.colRange(i, i + window_size).rowRange(y_pos, y_pos + window_size);
		if (ssd(window_size, left, temp) < ssd(window_size, left, cur_right)) {
			cur_right = temp;
			right_x_pos = i;
		}
	}
	return right_x_pos;
}

void fill_window_with_depth(int** depth_map, int window_size, int left_x_pos, int y_pos, Mat left_img, Mat right_img) {
	int right_x_pos = match_left_with_right(window_size, left_x_pos, y_pos, left_img, right_img);

	int disparity = abs(left_x_pos - right_x_pos);

	for (int i = y_pos; i < y_pos + window_size; i++) {
		for (int j = left_x_pos; j < left_x_pos + window_size; j++) {
			depth_map[i][j] = disparity;
		}
	}
}

void calculate_depth_map(int** depth_map, int window_size, Mat left_img, Mat right_img) {
	for (int i = 0; i < height - window_size; i += window_size) {
		for (int j = 0; j < width - window_size; j += window_size) {
			fill_window_with_depth(depth_map, window_size, j, i, left_img, right_img);
		}
		cout << "Window of height " << i << endl;
	}
}

void scale(int** depth_map) {		//Normalize depth map to range of 0 to 767

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (depth_map[i][j] > vmax) depth_map[i][j] = vmax;
			if (depth_map[i][j] < vmin) depth_map[i][j] = vmin;
			depth_map[i][j] = (double)depth_map[i][j] / vmax * 767;
			//cout << depth_map[i][j] << endl;
		}
	}
}
