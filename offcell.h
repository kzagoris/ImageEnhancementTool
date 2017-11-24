#pragma once

#include <stdio.h>
#include <cmath>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>





float AverageWindow(int x, int y, Mat& img, int W)
{
    //Mat img;
    //img_original.convertTo(img, CV_8U);
    int r = W / 2;
    float sum = 0;
    int count = 0;
    for (int yy = y - r; yy <= y + r; yy++)
    {
        if (yy < 0) continue;
        if (yy >= img.rows) continue;
        uchar* p = img.ptr<uchar>(yy);
        for (int xx = x - r; xx <= x + r; xx++)
        {
            if (xx < 0) continue;
            if (xx >= img.cols) continue;
            sum += p[xx];
            count++;
        }
    }
    return sum / count;
}

double getTheThreshVal_Otsu_8u( const cv::Mat& _src )
{
    cv::Size size = _src.size();
    if ( _src.isContinuous() )
    {
        size.width *= size.height;
        size.height = 1;
    }
    const int N = 256;
    int i, j, h[N] = {0};
    for ( i = 0; i < size.height; i++ )
    {
        const uchar* src = _src.data + _src.step*i;
        for ( j = 0; j <= size.width - 4; j += 4 )
        {
            int v0 = src[j], v1 = src[j+1];
            h[v0]++; h[v1]++;
            v0 = src[j+2]; v1 = src[j+3];
            h[v0]++; h[v1]++;
        }
        for ( ; j < size.width; j++ )
            h[src[j]]++;
    }

    double mu = 0, scale = 1./(size.width*size.height);
    for ( i = 0; i < N; i++ )
        mu += i*h[i];

    mu *= scale;
    double mu1 = 0, q1 = 0;
    double max_sigma = 0, max_val = 0;

    for ( i = 0; i < N; i++ )
    {
        double p_i, q2, mu2, sigma;

        p_i = h[i]*scale;
        mu1 *= q1;
        q1 += p_i;
        q2 = 1. - q1;

        if ( std::min(q1,q2) < FLT_EPSILON || std::max(q1,q2) > 1. - FLT_EPSILON )
            continue;

        mu1 = (mu1 + i*p_i)/q1;
        mu2 = (mu - q1*mu1)/q2;
        sigma = q1*q2*(mu1 - mu2)*(mu1 - mu2);
        if ( sigma > max_sigma )
        {
            max_sigma = sigma;
            max_val = i;
        }
    }

    return max_val;
}



float MeanSWValue(Mat& img)
{
    float sum = 0;
    int count = 0;
    for (auto it = img.begin<float>(), end = img.end<float>(); it != end; it++)
        if (*it > 0)
        {
            sum += *it;
            count++;
        }

    return count == 0 ? 0 : sum / count;
}



Mat SW(Mat& bw8u_orig)
{
    // bw8u : we want to calculate the SWT of this. NOTE: Its background pixels are 0 and foreground pixels are 1 (not 255!)
    Mat bw32f, swt32f, kernel;
    Mat bw8u;
    //threshold(bw8u_orig, bw8u, -1 , 1, THRESH_BINARY_INV | THRESH_OTSU);
	bw8u = Niblack(bw8u_orig);
    
    double max;
    int strokeRadius;

    bw8u.convertTo(bw32f, CV_32F);  // format conversion for multiplication
    distanceTransform(bw8u, swt32f, CV_DIST_L2, 5); // distance transform
    minMaxLoc(swt32f, NULL, &max);  // find max
    strokeRadius = (int)ceil(max);  // half the max stroke width
    kernel = getStructuringElement(MORPH_RECT, Size(3, 3)); // 3x3 kernel used to select 8-connected neighbors

    for (int j = 0; j < strokeRadius; j++)
    {
        dilate(swt32f, swt32f, kernel); // assign the max in 3x3 neighborhood to each center pixel
        swt32f = swt32f.mul(bw32f); // apply mask to restore original shape and to avoid unnecessary max propogation
    }
    return swt32f;
}






Mat EnhanceOffCellImage(Mat& inImg)
{
    double maxBrightness, minBrightness;
    minMaxLoc(inImg, &minBrightness, &maxBrightness);
    double otsuThreshold = getTheThreshVal_Otsu_8u(inImg);
    Mat enhanceImage = Mat::zeros(inImg.size(), CV_8U);
    uchar* in_img_p; uchar* en_img_p;
    for (int y = 0; y < inImg.rows; ++y) {
        in_img_p = inImg.ptr<uchar>(y);
        en_img_p = enhanceImage.ptr<uchar>(y);
        for (int x = 0; x < inImg.cols; ++x) {
            en_img_p[x] = (uchar)(in_img_p[x] < otsuThreshold ? otsuThreshold*(in_img_p[x] - minBrightness)/(otsuThreshold - minBrightness)  : in_img_p[x]);
        }
    }
    return enhanceImage;
}

Mat OffCellImageDynamic(Mat& inImg, int CL_WINDOW, int CS_WINDOW, float WS = 1.65f, float WL = 0.825f)
{
    Mat halfSWImg = SW(inImg);
    auto meanSWValue = MeanSWValue(halfSWImg);
    
    Mat offCell = Mat::ones(inImg.size(), CV_8U) * 255;

    for (int y = CL_WINDOW; y < inImg.rows - CL_WINDOW; y++)
    {
        uchar* p = offCell.ptr<uchar>(y);
        for (int x = CL_WINDOW; x < inImg.cols - CL_WINDOW; x++)
        {
            int sl_window = (int)ceil(2 *  halfSWImg.at<float>(y, x));
            if (sl_window == 0) continue;
            if (sl_window < 3)  sl_window = 3;
            int ss_window = (int)ceil(sl_window/3);
            
            float SL = AverageWindow(x, y, inImg, sl_window);
            float CL = AverageWindow(x, y, inImg, CL_WINDOW);
            float SS = AverageWindow(x, y, inImg, ss_window);
            float CS = AverageWindow(x, y, inImg, CS_WINDOW);

            float Surround = (SS + 10*SL) / 11;
            float Center = (10*CS + CL) / 11;

            float SC = Surround - Center;

            if (SC > 0)
                p[x] = (uchar)ceil(255 - (((255 + Surround)*SC) / (Surround + SC)));
        }
    }


    return EnhanceOffCellImage(offCell);

}






