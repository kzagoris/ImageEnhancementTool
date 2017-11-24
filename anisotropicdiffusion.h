#pragma once

#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace cv;

cv::Mat AnisotropicDiffusion(cv::Mat &io_img, int iter = 10, float K = 10)
{
    Mat x0;
    io_img.convertTo(x0, CV_32FC1);
    double t = 0;
    double lambda = 0.15; // Defined in equation (7)
    //double K = 10; // defined after equation(13) in text
    Mat x1, xc;

    for (int a = 0; a < iter; a++)
    {
	Mat D;		    // defined just before equation (5) in text
	Mat gradxX, gradyX; // Image Gradient t time
	Sobel(x0, gradxX, CV_32F, 1, 0, 1);
	Sobel(x0, gradyX, CV_32F, 0, 1, 1);
	D = Mat::zeros(x0.size(), CV_32F);
	for (int i = 0; i < x0.rows; i++)
	{
	    float *gradxX_p = gradxX.ptr<float>(i);
	    float *gradyX_p = gradyX.ptr<float>(i);
	    float *D_p = D.ptr<float>(i);
	    for (int j = 0; j < x0.cols; j++)
	    {

		float gx = gradxX_p[j], gy = gradyX_p[j];
		double d;
		if (i == 0 || i == x0.rows - 1 || j == 0 || j == x0.cols - 1) // conduction coefficient set to 1 p633 after equation 13
		    d = 1;
		else
		    d = 1.0 / (1.0 + abs((gx * gx + gy * gy)) / (K * K)); // expression of g(gradient(I))
									  //d =-exp(-(gx*gx + gy*gy)/(K*K)); // expression of g(gradient(I))
		D_p[j] = (float)d;
	    }
	}
	x1 = Mat::zeros(x0.size(), CV_32F);
	for (int i = 1; i < x0.rows - 1; i++)
	{
	    float *u1 = (float *)x1.ptr(i);
	    u1++;
	    for (int j = 1; j < x0.cols - 1; j++, u1++)
	    {
		// Value of I at (i+1,j),(i,j+1)...(i,j)
		float ip10 = x0.at<float>(i + 1, j), i0p1 = x0.at<float>(i, j + 1);
		float im10 = x0.at<float>(i - 1, j), i0m1 = x0.at<float>(i, j - 1), i00 = x0.at<float>(i, j);
		// Value of D at at (i+1,j),(i,j+1)...(i,j)
		float cp10 = D.at<float>(i + 1, j), c0p1 = D.at<float>(i, j + 1);
		float cm10 = D.at<float>(i - 1, j), c0m1 = D.at<float>(i, j - 1), c00 = D.at<float>(i, j);
		// Equation (7) p632
		*u1 = (float)(i00 + lambda / 4 * ((cp10 + c00) * (ip10 - i00) + (c0p1 + c00) * (i0p1 - i00) + (cm10 + c00) * (im10 - i00) + (c0m1 + c00) * (i0m1 - i00)));
		// equation (9)
	    }
	}
	x1.copyTo(x0);
	//x0.convertTo(xc, CV_8U);
	t = t + lambda;
    }
    x0.convertTo(xc, CV_8U);
    return xc;
}
