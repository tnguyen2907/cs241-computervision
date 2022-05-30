#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "main.h"
#include <float.h>
#include <cmath>

using namespace std;
using namespace cv;

double distance(Vec3b p1, Vec3b p2) {
	return sqrt(pow((int)p1[0] - (int)p2[0], 2) + pow((int)p1[1] - (int)p2[1], 2) + pow((int)p1[2] - (int)p2[2], 2));
}

double get_ssd(int window_size, Mat m1, Mat m2) {		//Sum of square difference
	double sum = 0;
	for (int i = 0; i < window_size; i++) {
		for (int j = 0; j < window_size; j++) {
			sum += pow(distance(m1.at<Vec3b>(i, j), m2.at<Vec3b>(i, j)), 2);
		}
	}
	return sum;
}

double match_cost(int window_size, Mat m1, Mat m2) {
	double ssd = get_ssd(window_size, m1, m2);
	double max = 3 * 255 * 255 * window_size * window_size;
	return ssd / max * 10000;
}

//int get_index(int i, int j, int k;) {
//	return vmax * (i * width + j) + k;
//}

double calculate_penalty(int** depth_map, int window_size, int left_x_pos, int right_x_pos, int y_pos) {
	double P1 = 15 ;		//Penalty
	double P2 = 30;

	int num_directions = 0;

	double w_cost = 0;		//West 
	double nw_cost = 0;		//North West
	double n_cost = 0;		//North
	double ne_cost = 0;		//North East

	if (y_pos != 0) {
		num_directions++;

		if (abs(abs(left_x_pos - right_x_pos) - depth_map[y_pos - window_size][left_x_pos]) == 1) n_cost = P1;
		else if (abs(abs(left_x_pos - right_x_pos) - depth_map[y_pos - window_size][left_x_pos]) > 1) n_cost = P2;
	}

	if (left_x_pos != 0) {
		num_directions++;

		if (abs(abs(left_x_pos - right_x_pos) - depth_map[y_pos][left_x_pos - window_size]) == 1) w_cost = P1;
		else if (abs(abs(left_x_pos - right_x_pos) - depth_map[y_pos][left_x_pos - window_size]) > 1) w_cost = P2;
	}

	if (left_x_pos != 0 && y_pos != 0) {
		num_directions++;

		if (abs(abs(left_x_pos - right_x_pos) - depth_map[y_pos - window_size][left_x_pos - window_size]) == 1) nw_cost = P1;
		else if (abs(abs(left_x_pos - right_x_pos) - depth_map[y_pos - window_size][left_x_pos - window_size]) > 1) nw_cost = P2;
	}

	if (left_x_pos < width - 2 * window_size && y_pos != 0) {
		num_directions++;

		if (abs(abs(left_x_pos - right_x_pos) - depth_map[y_pos - window_size][left_x_pos + window_size]) == 1) ne_cost = P1;
		else if (abs(abs(left_x_pos - right_x_pos) - depth_map[y_pos - window_size][left_x_pos + window_size]) > 1) ne_cost = P2;
	}

	if (num_directions == 0) return 0;
	return (w_cost + nw_cost + n_cost + ne_cost) / num_directions;
}

int match_left_with_right(int** depth_map, int window_size, int left_x_pos, int y_pos, Mat left_img, Mat right_img) {

	Mat left = left_img.colRange(left_x_pos, left_x_pos + window_size).rowRange(y_pos, y_pos + window_size);

	int start = left_x_pos - vmax;
	if (start < 0) start = 0;
	int end = left_x_pos;

	int right_x_pos = start;
	Mat cur_right = right_img.colRange(right_x_pos, right_x_pos + window_size).rowRange(y_pos, y_pos + window_size);
	double cur_energy = match_cost(window_size, left, cur_right) + calculate_penalty(depth_map, window_size, left_x_pos, right_x_pos, y_pos);

	for (int i = start; i < end; i += window_size) {
		Mat temp = right_img.colRange(i, i + window_size).rowRange(y_pos, y_pos + window_size);
		double temp_energy = match_cost(window_size, left, temp) + calculate_penalty(depth_map, window_size, left_x_pos, i, y_pos);

		if (temp_energy < cur_energy) {
			cur_energy = temp_energy;
			right_x_pos = i;
		}
	}
	return right_x_pos;
}

void fill_window_with_depth(int** depth_map, int window_size, int left_x_pos, int y_pos, Mat left_img, Mat right_img) {
	int right_x_pos = match_left_with_right(depth_map, window_size, left_x_pos, y_pos, left_img, right_img);

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

void scale(int** depth_map) {

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (depth_map[i][j] > vmax) depth_map[i][j] = vmax;
			if (depth_map[i][j] < vmin) depth_map[i][j] = vmin;
			depth_map[i][j] = (double)depth_map[i][j] / vmax * 767;
			//cout << depth_map[i][j] << endl;
		}
	}
}

