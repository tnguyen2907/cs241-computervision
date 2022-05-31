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
#include <Eigen/Dense>
//#include <Eigen/SVD> 

using namespace cv;
using namespace std;
using namespace Eigen;

Mat lhs_mat(Point arr1[9], Point arr2[9]) {
    Mat lhs = Mat::zeros(9, 9, CV_64FC1);
    for (int i = 0; i < 9; i++) {
        lhs.at<double>(i, 0) += ((int)arr1[i].x) * ((int)arr2[i].x);
        lhs.at<double>(i, 1) += ((int)arr1[i].x) * ((int)arr2[i].y);
        lhs.at<double>(i, 2) += (int)arr1[i].x;
        lhs.at<double>(i, 3) += ((int)arr1[i].y) * ((int)arr2[i].x);
        lhs.at<double>(i, 4) += ((int)arr1[i].y) * ((int)arr2[i].y);
        lhs.at<double>(i, 5) += (int)arr1[i].y;
        lhs.at<double>(i, 6) += (int)arr2[i].x;
        lhs.at<double>(i, 7) += (int)arr2[i].y;
        lhs.at<double>(i, 8) += 1;
    }
    return lhs;
}


Mat fundamental(Point arr1[9], Point arr2[9]) {
    Mat lhs = lhs_mat(arr1, arr2);
    Mat sub_fund = Mat::zeros(9, 1, CV_64FC1);
    SVD::solveZ(lhs, sub_fund);
    Mat fund = Mat::zeros(3, 3, CV_64FC1);
    fund.at<double>(0, 0) += sub_fund.at<double>(0, 0);
    fund.at<double>(0, 1) += sub_fund.at<double>(1, 0);
    fund.at<double>(0, 2) += sub_fund.at<double>(2, 0);
    fund.at<double>(1, 0) += sub_fund.at<double>(3, 0);
    fund.at<double>(1, 1) += sub_fund.at<double>(4, 0);
    fund.at<double>(1, 2) += sub_fund.at<double>(5, 0);
    fund.at<double>(2, 0) += sub_fund.at<double>(6, 0);
    fund.at<double>(2, 1) += sub_fund.at<double>(7, 0);
    fund.at<double>(2, 2) += sub_fund.at<double>(8, 0);
    return fund;
}

int main(int argc, char** argv)
{
    Point key1[9] = { Point(0, 0) , Point(1, 6), Point(8, 4),
                    Point(6, 27), Point(13, 9), Point(5, 2),
                    Point(72, 86), Point(5, 94), Point(56, 23) };
    Point key2[9] = { Point(10, 0) , Point(2, 4), Point(17, 3),
                    Point(16, 25), Point(12, 37), Point(3, 8),
                    Point(46, 81), Point(7, 100), Point(43, 19) };
    Mat fund = fundamental(key1, key2);
    return 0;
}

// Estimate the fundamental matrix including normalize the points
void fund_eight_pts(vector<Vec2f>& x1, vector<Vec2f>& x2, Mat3f& F)
{
    // Create 2 vectors for normalization
    vector<Vec2f> nx1, nx2;
    // Create 2 matrix 3x3
    Mat3f T1, T2;
    // Normalize the points x1, x2
    T1 = normalize_pts(x1, nx1);
    T2 = normalize_pts(x2, nx2);

    // Create 2 matrix
    Mat matx1, matx2;
    // Change the dimension of the matrix
    vector2mat<float>(nx1, matx1);
    vector2mat<float>(nx2, matx2);

    // Add numbers for matrix A
    Mat A(9, x1.size());
    A << matx2.row(0).array() * matx1.row(0).array(),
        matx2.row(0).array()* matx1.row(1).array(),
        matx2.row(0).array(),
        matx2.row(1).array()* matx1.row(0).array(),
        matx2.row(1).array()* matx1.row(1).array(),
        matx2.row(1).array(),
        matx1.row(0),
        matx1.row(1),
        Mat::Ones(1, x1.size());
    A = A.transpose().eval();

    // Solve the constraint equation for F from nullspace extraction
    // An LU decomposition is efficient for the minimally constrained case
    // Otherwise, use an SVD
    vector<Vec9f> fvector;
    if (0)
    {
        const auto lu_decomp = A.fullPivLu();
        if (lu_decomp.dimensionOfKernel() == 1)
        {
            fvector = lu_decomp.kernel();
        }
    }
    
    else
    {
        JacobiSVD<Eigen::Matrix<float, Eigen::Dynamic, 9> > amatrix_svd(A, Eigen::ComputeFullV);
        fvector = amatrix_svd.matrixV().col(8);
    }
    
    // Rearrange the vectors to forms the matrix
    F << fvector(0), fvector(1), fvector(2),
        fvector(3), fvector(4), fvector(5),
        fvector(6), fvector(7), fvector(8);

    // enforce the constraint that F is of rank 2
    // Find the closest singular matrix to F under frobenius norm
    // We can compute this matrix with SVD
    JacobiSVD<Mat3f> fmatrix_svd(F, Eigen::ComputeFullU | Eigen::ComputeFullV);
    Vec3f singular_values = fmatrix_svd.singularValues();
    singular_values(2) = 0.0f;
    F = fmatrix_svd.matrixU() * singular_values.asDiagonal() * fmatrix_svd.matrixV().transpose();

    F = T2.transpose() * F * T1;
}

// Normalize the points
Mat3f normalize_pts(vector<Vec2f>& pts, vector<Vec2f>& newpts)
{
    newpts.resize(pts.size());
    Vec2f c(0, 0);
    for (int i = 0; i < pts.size(); ++i)
    {
        c += pts[i];
    }
    c /= (int) pts.size();

    float dist = 0.0f;
    for (int i = 0; i < pts.size(); ++i)
    {
        newpts[i] = pts[i] - c;
        dist += norm(newpts[i]);
    }
    dist /= pts.size();

    float scale = sqrt(2) / dist;
    for (int i = 0; i < pts.size(); ++i)
    {
        newpts[i] *= scale;
    }

    Mat3f T;
    T << scale, 0, -scale * c(0),
        0, scale, -scale * c(1),
        0, 0, 1;

    return T;
}
