#ifndef HEADER_H
#define HEADER_H

extern int baseline;
extern int width;
extern int height;


//Print output
void print_depth_map(int** depth_map);
//Get key points
cv::Vec2b* get_key_pts(cv::Mat img);
//Match key points
cv::Vec2b* match_key_pts(cv::Vec2b* left_key_pts, cv::Vec2b* right_key_pts, cv::Mat left_img, cv::Mat right_img);

#endif