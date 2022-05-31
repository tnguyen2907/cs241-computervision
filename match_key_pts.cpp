#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "main.h"

using namespace std;
using namespace cv;

double get_ssd(Mat m1, Mat m2) {		//Sum of square difference
	double sum = 0;
	for (int i = 0; i < 128; i++) {
		sum += pow(m1.at<float>(0, 0) - m2.at<float>(0, 0), 2);
	}
	return sum;
}

void match(vector<KeyPoint> left_key_pts, vector<KeyPoint> right_key_pts, Point left_pts[9], Point right_pts[9], int left_index[9], int right_index[9]) {
	for (int i = 0; i < 9; i++) {
		left_pts[i] = left_key_pts[left_index[i]].pt;
	}
	for (int i = 0; i < 9; i++) {
		right_pts[i] = right_key_pts[right_index[i]].pt;
	}
}


void match_key_pts(vector<Mat> left_descriptors, vector<Mat> right_descriptors, vector <KeyPoint> left_key_pts, vector<KeyPoint> right_key_pts, Point left_pts[9], Point right_pts[9]) {
	int left_index[9];
	for (int i = 0; i < 9; i++) left_index[i] = 0;
	int right_index[9];
	for (int i = 0; i < 9; i++) right_index[i] = 0;

	double ssd_lst[9];

	int n = 0;
	for (int i = 0; i < left_descriptors.size(); i++) {
		cout << i << endl;
		int min_index = 0;
		double min_ssd = get_ssd(left_descriptors[i], right_descriptors[0]);
		for (int j = 0; j < right_descriptors.size(); j++) {
			bool con = false;
			for (int k = 0; k < n; k++) {
				if (right_index[k] == j) con = true;
			}
			if (con) continue;
			double cur_ssd = get_ssd(left_descriptors[i], right_descriptors[j]);
			if (cur_ssd < min_ssd) {
				min_ssd = cur_ssd;
				min_index = j;
			}
		}
		if (n != 9) {
			ssd_lst[n] = min_ssd;
			left_index[n] = i;
			right_index[n] = min_index;
			n++;
		} else {
			int max_index = 0;
			for (int k = 0; k < 9; k++) {
				if (ssd_lst[k] > ssd_lst[max_index]) max_index = k;
			}

			if (min_ssd < ssd_lst[max_index]) {
				ssd_lst[max_index] = min_ssd;
				left_index[max_index] = i;
				right_index[max_index] = min_index;
			}
		}
	}
	match(left_key_pts, right_key_pts, left_pts, right_pts, left_index, right_index);
}


