#ifndef HEADER_H
#define HEADER_H

extern std::string folder_path;
extern int width;
extern int height;
extern int vmin;
extern int vmax;

//Print output
void print_depth_map(int** depth_map);

//Get key points
std::tuple<std::vector<cv::KeyPoint>,std::vector<cv::Mat>> get_key_pts(cv::Mat img);

//Match key points
void match_key_pts(std::vector<cv::Mat> left_descriptors, std::vector<cv::Mat> right_descriptors, std::vector<cv::KeyPoint> left_key_pts, std::vector<cv::KeyPoint> right_key_pts, cv::Point left_pts[8], cv::Point right_pts[8]);

//Calculate fundamental matrix
cv::Mat fundamental(cv::Point arr1[9], cv::Point arr2[9]);

//Rotate image
std::tuple<cv::Mat,cv::Mat> rectify(cv::Mat img1, cv::Mat img2, cv::Mat fund);

//Calculate depth map using sum of square difference and window matching
void calculate_depth_map(int** depth_map, int window_size, cv::Mat left_img, cv::Mat right_img);

//Processed depth map so it is easy to see
void scale(int** depth_map);

#endif