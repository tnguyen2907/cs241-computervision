#define _USE_MATH_DEFINES

#include <stdio.h>
#include <cmath>
#include <string> 
#include <iostream>
#include <algorithm>
#include <tuple>
#include <opencv2\opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2/features2d.hpp>

using namespace cv;
using namespace std;

typedef struct {
    Point2f pt;
    float size;
    float angle;
    int octave;
} keypoint;

// gamma correction for image before gaussian blur
void gamma_correction(Mat org, Mat ret, float gamma) {
    int org_height = org.size().height;
    int org_width = org.size().width;
    unsigned char lut[256];
    for (int i = 0; i < 256; i++) {
        lut[i] = saturate_cast<uchar>(pow((float)(i / 255.0), gamma) * 255.0f);
    }
    ret = org.clone();
    for (int i = 0; i < org_width; i++) {
        for (int j = 0; j < org_height; j++) {
            auto color = org.at<uchar>(j, i);
            uchar ret_color(lut[color]);
            ret.at<uchar>(j, i) = ret_color;
        }
    }
}

// add padding around the image
void addPadding(Mat org, Mat ret, int padding) {
    int org_height = org.size().height;
    int org_width = org.size().width;
    if (padding > org_height || padding > org_width) return;
    int width = org_width + 2 * padding;
    int height = org_height + 2 * padding;
    ret = Mat(height, width, CV_8UC1, Scalar(0, 0, 0));
    for (int i = 0; i < padding; i++) {
        for (int j = 0; j < padding; j++) {
            ret.at<uchar>(j, i) = org.at<uchar>(padding - j, padding - i);
        }
        for (int j = padding; j < org_height + padding; j++) {
            ret.at<uchar>(j, i) = org.at<uchar>(j - padding, padding - i);
        }
        for (int j = org_height + padding; j < height; j++) {
            ret.at<uchar>(j, i) = org.at<uchar>(j - 2 * padding, padding - i);
        }
    }
    for (int i = org_width + padding; i < width; i++) {
        for (int j = 0; j < padding; j++) {
            ret.at<uchar>(j, i) = org.at<uchar>(padding - j, i - 2 * padding);
        }
        for (int j = padding; j < org_height + padding; j++) {
            ret.at<uchar>(j, i) = org.at<uchar>(j - padding, i - 2 * padding);
        }
        for (int j = org_height + padding; j < height; j++) {
            ret.at<uchar>(j, i) = org.at<uchar>(j - 2 * padding, i - 2 * padding);
        }
    }
    for (int i = padding; i < org_width + padding; i++) {
        for (int j = 0; j < height; j++) {
            if (j < padding) {
                ret.at<uchar>(j, i) = org.at<uchar>(padding - j, i - padding);
            }
            else if (j < org_height + padding) {
                ret.at<uchar>(j, i) = org.at<uchar>(j - padding, i - padding);
            }
            else {
                ret.at<uchar>(j, i) = org.at<uchar>(j - 2 * padding, i - padding);
            }
        }
    }
}

// calculate how many octave an image will have
int wave_num(Size image_shape) {
    return int(round(log(min(image_shape.height, image_shape.width)) / log(2) - 1));
}

// generate sigmas for every octave
vector<double> octave_sigmas(double sigma) {
    vector<double> sigmas;
    double scale = pow(2, 1.0 / 3.0);
    sigmas.push_back(sigma);
    for (int i = 1; i < 6; i++) {
        double sig = pow(scale, i) * sigma;
        sigmas.push_back(sig);
    }
    return sigmas;
}

// resize the image to half
void halfimg(Mat org, Mat ret) {
    int org_width = org.size().width;
    int org_height = org.size().height;
    int width = org_width / 2;
    int height = org_height / 2;
    ret = Mat::zeros(height, width, CV_32FC1);
    int new_width = 0;
    for (int i = 0; i < org_width - 1; i += 2)
    {
        int new_height = 0;
        for (int j = 0; j < org_height - 1; j += 2)
        {
            ret.at<float>(new_height, new_width) = org.at<float>(j, i);
            new_height++;
        }
        new_width++;
    }
}

// double the size of the image
void doubleimg(Mat org, Mat ret) {
    int org_width = org.size().width;
    int org_height = org.size().height;
    int width = org_width * 2;
    int height = org_height * 2;
    ret = Mat::zeros(height, width, CV_32FC1);
    int new_width = 0;
    for (int i = 0; i < org_width - 1; i++)
    {
        for (int j = 0; j < org_height - 1; j++)
        {
            ret.at<float>(2 * j, 2 * i) = org.at<float>(j, i);
            ret.at<float>(2 * j + 1, 2 * i) = org.at<float>(j, i);
            ret.at<float>(2 * j, 2 * i + 1) = org.at<float>(j, i);
            ret.at<float>(2 * j + 1, 2 * i + 1) = org.at<float>(j, i);
        }
    }
}

// calculate the gradient based on 3 matrix 3x3
Mat compute_gradient(Mat mid, Mat next, Mat prev) {
    double dx = 0.5 * (mid.at<float>(1, 2) - mid.at<float>(1, 0));
    double dy = 0.5 * (mid.at<float>(2, 1) - mid.at<float>(0, 1));
    double dsigma = 0.5 * (next.at<float>(1, 1) - prev.at<float>(1, 1));
    Mat gradient = (Mat_<float>(3, 1) << dx, dy, dsigma);
    return gradient;
}

// calculate the hessian matrix based on 3 matrix 3x3
Mat compute_hessian(Mat mid, Mat next, Mat prev) {
    double dxx, dyy, dzz, dxy, dxz, dyz;
    dxx = mid.at<float>(1, 2) - 2 * mid.at<float>(1, 1) + mid.at<float>(1, 0);
    dyy = mid.at<float>(2, 1) - 2 * mid.at<float>(1, 1) + mid.at<float>(0, 1);
    dzz = next.at<float>(1, 1) - 2 * mid.at<float>(1, 1) + prev.at<float>(1, 1);
    dxy = 0.25 * (mid.at<float>(2, 2) - mid.at<float>(0, 2) - mid.at<float>(2, 0) + mid.at<float>(0, 0));
    dxz = 0.25 * (next.at<float>(1, 2) - prev.at<float>(1, 2) - next.at<float>(1, 0) + prev.at<float>(1, 0));
    dyz = 0.25 * (next.at<float>(2, 1) - prev.at<float>(2, 1) - next.at<float>(0, 1) + prev.at<float>(0, 1));
    Mat hessian = (Mat_<float>(3, 3) << dxx, dxy, dxz, dxy, dyy, dyz, dxz, dyz, dzz);
    return hessian;
}

// generate the kernel for a sigma
Mat gaussian_kernel(double sigma) {
    int r = (int)ceil(3 * sigma);
    Mat kernel(2 * r + 1, 2 * r + 1, CV_64FC1);
    for (int i = -r; i <= r; i++)
    {
        for (int j = -r; j <= r; j++)
        {
            kernel.at<double>(i + r, j + r) = exp(-(i * i + j * j) / (2.0 * sigma * sigma));
        }
    }
    kernel = kernel / sum(kernel);
    return kernel;
}

///generate a list of scale space and a list of Difference of Gaussian
tuple<vector<vector<Mat>>, vector<vector<Mat>>> dog(Mat org, double sigma) {
    int waves = wave_num(org.size());
    vector<double> sigmas = octave_sigmas(sigma);
    vector<vector<Mat>> scale_space;
    vector<vector<Mat>> dog;
    for (int i = 0; i < waves; i++)
    {
        scale_space.push_back(vector<Mat>(6));
        dog.push_back(vector<Mat>(5));
    }
    GaussianBlur(org, scale_space[0][0], Size(0, 0), sigmas[0], sigmas[0]);
    for (int octave = 0; octave < waves; octave++) {
        for (int layer = 1; layer < 6; layer++) {
            GaussianBlur(scale_space[octave][layer - 1], scale_space[octave][layer], Size(0, 0), sigmas[layer], sigmas[layer]);
            dog[octave][layer - 1] = scale_space[octave][layer] - scale_space[octave][layer - 1];
            if (octave == waves - 1) continue;
            halfimg(scale_space[octave][0], scale_space[octave + 1][0]);
        }
    }
    return make_tuple(scale_space, dog);
}

// check if a pixel is an extrema in 3 difference matrix 3x3
bool extrema(Mat mid, Mat prevMat, Mat nextMat) {
    if (mid.at<float>(1, 1) < 0.03) return false;
    double min, max;
    Point minLoc, maxLoc;
    minMaxLoc(mid, &min, &max, &minLoc, &maxLoc);
    if ((minLoc.x != 1 || minLoc.y != 1) && (maxLoc.x != 1 || maxLoc.y != 1)) return false;
    double minPrev, maxPrev, minNext, maxNext;
    Point minLocPrev, maxLocPrev;
    Point minLocNext, maxLocNext;
    minMaxLoc(prevMat, &minPrev, &maxPrev, &minLocPrev, &maxLocPrev);
    minMaxLoc(nextMat, &minNext, &maxNext, &minLocNext, &maxLocNext);
    if ((minLoc.x == 1 && minLoc.y == 1) && (min > minPrev || min > minNext)) return false;
    if ((maxLoc.x == 1 && maxLoc.y == 1) && (max < maxPrev || max < maxNext)) return false;
    return true;
}

// localize the point of the scale space to the original image
keypoint localize(int i, int j, vector<Mat> octave, int layer, int index) {
    keypoint ret;
    ret.pt = Point2f(-1, -1);
    ret.size = 0;
    int change = 1;
    for (int c = 0; c < 5; c++) {
        Mat mid = octave[layer].clone();
        Mat nextMat = octave[layer + 1].clone();
        Mat prevMat = octave[layer - 1].clone();
        if (change) {
            normalize(mid, mid, 0, 1, NORM_MINMAX);
            normalize(nextMat, nextMat, 0, 1, NORM_MINMAX);
            normalize(prevMat, prevMat, 0, 1, NORM_MINMAX);
            change = 0;
        }

        Mat grad = compute_gradient(mid(Rect(j - 1, i - 1, 3, 3)), nextMat(Rect(j - 1, i - 1, 3, 3)), prevMat(Rect(j - 1, i - 1, 3, 3)));

        Mat hessian = compute_hessian(mid(Rect(j - 1, i - 1, 3, 3)), nextMat(Rect(j - 1, i - 1, 3, 3)), prevMat(Rect(j - 1, i - 1, 3, 3)));

        Mat extreme_loc;
        solve(hessian, grad, extreme_loc);
        if (abs(extreme_loc.at<float>(0)) < 0.5 && abs(extreme_loc.at<float>(1)) < 0.5 && abs(extreme_loc.at<float>(2)) < 0.5) {
            if (mid.at<float>(i, j) < 0.03) return ret;
            double trH = hessian.at<float>(0, 0) + hessian.at<float>(1, 1);
            double detH = hessian.at<float>(0, 0) * hessian.at<float>(1, 1) - hessian.at<float>(0, 1) * hessian.at<float>(1, 0);
            double ratio = trH * trH / detH;
            if (detH < 0 || ratio > 10) return ret;
            ret.pt.x = j * pow(2, index);
            ret.pt.y = i * pow(2, index);
            ret.size = layer;
            return ret;
        }
        i -= int(round(extreme_loc.at<float>(1)));
        j -= int(round(extreme_loc.at<float>(0)));
        layer -= int(round(extreme_loc.at<float>(2)));
        if (i < 5 || i > mid.size().height - 5 || j < 5 || j > mid.size().width - 5 || layer < 1 || layer >= octave.size() - 1) return ret;
        if (int(round(extreme_loc.at<float>(2)))) change = 1;
    }
    return ret;
}

// compute the orientation of each key point
vector<float> orientation(Point P, int octave, double sigma, Mat org) {
    vector<float> orien;
    Mat kernel = gaussian_kernel(sigma);

    int radius = int(2 * ceil(sigma) + 1);
    double weight;
    Mat histogram = Mat::zeros(36, 1, CV_64FC1);
    for (int i = -radius; i <= radius; i++) {
        int y = int(round((P.y / pow(2, octave)))) + i;
        if (y <= 0 || y >= org.rows - 1) continue;

        for (int j = -radius; j <= radius; j++) {
            int x = int(round((P.x / pow(2, octave)))) + j;
            if (x <= 0 || x >= org.cols - 1) continue;

            double dx = org.at<float>(y, x + 1) - org.at<float>(y, x - 1);
            double dy = org.at<float>(y + 1, x) - org.at<float>(y - 1, x);
            double magnitude = sqrt(dx * dx + dy * dy);
            double orientation = atan2(dy, dx) * (180.0 / M_PI);

            if (orientation < 0) orientation += 360;
            int hist_index = int(floor(orientation * 36.0 / 360.0));
            weight = kernel.at<double>(j + radius, i + radius) * magnitude;
            histogram.at<double>(hist_index, 0) += weight;
        }
    }

    double max;
    Point maxLoc;
    double left_value, right_value, orientation, interpolated_index;
    minMaxLoc(histogram, NULL, &max, NULL, &maxLoc);
    for (int bin = 0; bin < histogram.rows; bin++)
    {
        if (histogram.at<double>(bin, 0) >= (0.8 * max))
        {
            if (bin == 0) left_value = histogram.at<double>(35, 0);
            else left_value = histogram.at<double>(bin - 1, 0);

            if (bin == 35) right_value = histogram.at<double>(0, 0);
            else right_value = histogram.at<double>(bin + 1, 0);

            interpolated_index = bin + 0.5 * (left_value - right_value) / (left_value - 2 * histogram.at<double>(bin, 0) + right_value);
            orientation = interpolated_index * 360.0 / 36.0;
            if (orientation < 0 || orientation >= 360) orientation = abs(360 - abs(orientation));
            orien.push_back(orientation);
        }
    }
    return orien;
}

// function to sort the key point
bool compareKey(KeyPoint k1, KeyPoint k2) {
    if (k1.pt.x != k2.pt.x) {
        return (k1.pt.x < k2.pt.x);
    }
    else {
        return (k1.pt.y < k2.pt.y);
    }
}

// remove duplicate keypoint
vector<KeyPoint> unique(vector<KeyPoint> keypoints) {
    vector<KeyPoint> unique_key;
    if (keypoints.size() <= 2) return keypoints;
    sort(keypoints.begin(), keypoints.end(), compareKey);
    unique_key.push_back(keypoints[0]);
    for (KeyPoint key : keypoints) {
        if (unique_key.back().pt.x != key.pt.x || unique_key.back().pt.y != key.pt.y) unique_key.push_back(key);
    }
    keypoints.clear();
    return unique_key;
}

// get key point from the Difference of Gaussian list
vector<KeyPoint> get_keypoint(vector<vector<Mat>> scale_space, vector<vector<Mat>> dog_scale) {
    vector<KeyPoint> list;
    for (int octave = 0; octave < dog_scale.size(); octave++) {
        printf("octave %d\n", octave);
        for (int layer = 1; layer < dog_scale[0].size() - 1; layer++) {
            int end_w = dog_scale[octave][0].size().width - 5;
            int end_h = dog_scale[octave][0].size().height - 5;
            int cnt = 0;
            cout << "looking at layer " << layer << endl;
            for (int i = 5; i < end_h; i++) {
                for (int j = 5; j < end_w; j++) {
                    keypoint current;
                    current.size = 0;
                    Mat mid = dog_scale[octave][layer](Rect(j - 1, i - 1, 3, 3)).clone();
                    Mat prevMat = dog_scale[octave][layer - 1](Rect(j - 1, i - 1, 3, 3)).clone();
                    Mat nextMat = dog_scale[octave][layer + 1](Rect(j - 1, i - 1, 3, 3)).clone();
                    if (extrema(mid, prevMat, nextMat)) {
                        cnt++;
                        current = localize(i, j, dog_scale[octave], layer, octave);
                    }
                    if (current.size) {
                        vector<float> orientations = orientation(current.pt, octave, layer * 1.5, scale_space[octave][current.size]);
                        for (float angle : orientations)
                        {
                            KeyPoint tmp(current.pt, current.size, angle, 0, octave);
                            // cout << "x: " << tmp.pt.x << ", y: " << tmp.pt.y << " with size " << tmp.size << ", angle of " << tmp.angle << " at octave " << tmp.octave << endl;
                            list.push_back(tmp);
                        }
                        orientations.clear();
                    }
                }
            }
            printf("have %d keypoints at layer %d\n", cnt, layer);
        }
    }
    return unique(list);
}

// get descriptors from each keypoint
vector<Mat> descriptor(vector<KeyPoint> key, vector<vector<Mat>> scale_space) {
    Mat kernel = gaussian_kernel(16 / 6.0);
    int pad = 8;
    vector<Mat> feature;
    Mat feature_vec = Mat::zeros(128, 1, CV_64FC1);
    for (KeyPoint p : key) {
        Mat current = scale_space[p.octave][p.size];
        int x = (int)p.pt.x / pow(2, p.octave);
        int y = (int)p.pt.y / pow(2, p.octave);

        Mat magnitude = Mat::zeros(17, 17, CV_64FC1);
        Mat orien = Mat::zeros(17, 17, CV_64FC1);

        if (x <= pad || y <= pad) continue;
        if (x + pad >= current.cols - 1 || y + pad >= current.rows - 1) continue;
        for (int i = -pad; i <= pad; i++) {
            int iloc = y + i;
            for (int j = -pad; j <= pad; j++) {
                int jloc = x + j;
                double dx = current.at<float>(jloc, iloc + 1) - current.at<float>(jloc, iloc - 1);
                double dy = current.at<float>(jloc + 1, iloc) - current.at<float>(jloc - 1, iloc);
                magnitude.at<double>(i + pad, j + pad) = sqrt(dx * dx + dy * dy);
                double theta = atan2(dy, dx) * (180.0 / M_PI);
                if (theta < 0) theta += 360;
                orien.at<double>(i + pad, j + pad) = theta;
            }
        }
        Mat weighted_grad = magnitude.mul(kernel);
        for (int i = 0; i <= 13; i = i + 4)
        {
            int m = 0;
            for (int j = 0; j <= 13; j = j + 4)
            {
                Mat tmp = orien(Rect(i, j, 4, 4));
                for (int a = 0; a < tmp.rows; a++)
                {
                    for (int b = 0; b < tmp.cols; b++)
                    {
                        int value = floor(tmp.at<float>(a, b) / 45.0);
                        feature_vec.at<float>(m + value) += 1.0;
                    }
                }
                m += 8;
                if (j == 4) j += 1;
            }
            if (i == 4) i += 1;
        }
        feature_vec = feature_vec / max(1e-6, norm(feature_vec, NORM_L2));
        threshold(feature_vec, feature_vec, 0.2, 255, THRESH_TRUNC);
        feature_vec = feature_vec / max(1e-6, norm(feature_vec, NORM_L2));
        feature.push_back(feature_vec);
    }
    return feature;
}

tuple<vector<KeyPoint>, vector<Mat>> get_key_pts(Mat img) {
    Mat output;
    cvtColor(img, output, COLOR_BGR2GRAY);
    Ptr<SiftFeatureDetector> detector = SiftFeatureDetector::create();
    vector<KeyPoint> key_before;
    Mat des;
    detector->detect(output, key_before);
    vector<KeyPoint> keypoints = unique(key_before);
    detector->compute(output, keypoints, des);
    vector<Mat> descriptors;
    for (int i = 0; i < des.rows; i++)
        descriptors.push_back(des.row(i));
    return make_tuple(keypoints, descriptors);
}