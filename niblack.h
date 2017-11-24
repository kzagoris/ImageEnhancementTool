#pragma once
#include <tuple>

using namespace std;


Mat Niblack(Mat& InputImg, float k = -2, int dW = 30)
{


	int Ix = InputImg.cols;
	int Iy = InputImg.rows;
	unsigned char *inputIm = NULL;
	inputIm = (unsigned char*)malloc(Ix*Iy * sizeof(unsigned char));
	for (int y = 0; y < Iy; y++)
	{
		auto v = InputImg.ptr<uchar>(y);
		for (int x = 0; x < Ix; x++)
			*(inputIm + y*Ix + x) = v[x];
	}
	unsigned char *outputImB = NULL;
	outputImB = (unsigned char*)calloc(Ix*Iy, sizeof(unsigned char));


	long *IX_pix, *IX_gray, *IX_graygray, *IX_pix1, *IX_gray1, *IX_graygray1;

	IX_pix = NULL;
	IX_gray = NULL;
	IX_graygray = NULL;
	IX_pix1 = NULL;
	IX_gray1 = NULL;
	IX_graygray1 = NULL;

	IX_pix = (long *)calloc((Ix + 2), sizeof(long));


	IX_gray = (long *)calloc((Ix + 2), sizeof(long));


	IX_graygray = (long *)calloc((Ix + 2), sizeof(long));


	IX_gray1 = (long *)calloc((Ix + 2), sizeof(long));



	IX_graygray1 = (long *)calloc((Ix + 2), sizeof(long));



	IX_pix1 = (long *)calloc((Ix + 2), sizeof(long));



	for (int y = 0; y < Iy; y++)
	{
		if (y == 0)
		{
			for (int x = 0; x < Ix; x++)
			{
				long pix = 0; long gray = 0; long gray2 = 0;
				for (int iy = 0; iy <= dW; iy++)
				{
					pix++;
					{
						gray += *(inputIm + iy*Ix + x);
						gray2 += (*(inputIm + iy*Ix + x))*(*(inputIm + iy*Ix + x));
					}
				}
				IX_pix[x] = pix; IX_gray[x] = gray; IX_graygray[x] = gray2;
				IX_pix1[x] = IX_pix[x]; IX_gray1[x] = IX_gray[x]; IX_graygray1[x] = IX_graygray[x];
			}
		}
		else
			if (y <= dW)
			{
				for (int x = 0; x < Ix; x++)
				{
					IX_pix[x] = IX_pix1[x] + 1;
					{
						IX_gray[x] = IX_gray1[x] + *(inputIm + (y + dW)*Ix + x);
						IX_graygray[x] = IX_graygray1[x] + (*(inputIm + (y + dW)*Ix + x))*(*(inputIm + (y + dW)*Ix + x));
					}
					IX_pix1[x] = IX_pix[x]; IX_gray1[x] = IX_gray[x]; IX_graygray1[x] = IX_graygray[x];
				}
			}
			else
				if (y <= Iy - 1 - dW)
				{
					for (int x = 0; x < Ix; x++)
					{
						IX_pix[x] = IX_pix1[x];
						{
							IX_gray[x] = IX_gray1[x] + *(inputIm + (y + dW)*Ix + x) - *(inputIm + (y - dW - 1)*Ix + x);
							IX_graygray[x] = IX_graygray1[x] + (*(inputIm + (y + dW)*Ix + x))*(*(inputIm + (y + dW)*Ix + x)) - (*(inputIm + (y - dW - 1)*Ix + x))*(*(inputIm + (y - dW - 1)*Ix + x));
						}
						IX_pix1[x] = IX_pix[x]; IX_gray1[x] = IX_gray[x]; IX_graygray1[x] = IX_graygray[x];
					}
				}
				else
				{
					for (int x = 0; x < Ix; x++)
					{
						IX_pix[x] = IX_pix1[x] - 1;
						{
							IX_gray[x] = IX_gray1[x] - (*(inputIm + (y - dW - 1)*Ix + x));
							IX_graygray[x] = IX_graygray1[x] - (*(inputIm + (y - dW - 1)*Ix + x))*(*(inputIm + (y - dW - 1)*Ix + x));
						}
						IX_pix1[x] = IX_pix[x]; IX_gray1[x] = IX_gray[x]; IX_graygray1[x] = IX_graygray[x];
					}
				}

		long pix1, gray1, graygray1;
		for (int x = 0; x < Ix; x++)
		{
			float m, s;
			long TH, pix, gray, graygray;

			if (x == 0)
			{
				pix = 0; gray = 0; graygray = 0;
				for (int ix = 0; ix <= dW; ix++)
				{
					pix += IX_pix[ix]; gray += IX_gray[ix]; graygray += IX_graygray[ix];
				}
			}
			else
				if (x <= dW)
				{
					pix = pix1 + IX_pix[x + dW];
					gray = gray1 + IX_gray[x + dW];
					graygray = graygray1 + IX_graygray[x + dW];
				}
				else
					if (x <= Ix - 1 - dW)
					{
						pix = pix1 + IX_pix[x + dW] - IX_pix[x - dW - 1];
						gray = gray1 + IX_gray[x + dW] - IX_gray[x - dW - 1];
						graygray = graygray1 + IX_graygray[x + dW] - IX_graygray[x - dW - 1];
					}
					else
					{
						pix = pix1 - IX_pix[x - dW - 1];
						gray = gray1 - IX_gray[x - dW - 1];
						graygray = graygray1 - IX_graygray[x - dW - 1];
					}

			pix1 = pix; gray1 = gray; graygray1 = graygray;

            m = gray /(float) pix;
			s = sqrt(fabs((float)(pix*graygray - gray*gray) / (pix*(pix - 1))));

            TH = (long)( m + (float)(k*s) / 10);
			{
				if ((*(inputIm + y*Ix + x) >= TH))
					*(outputImB + y*Ix + x) = 0;
				else
					*(outputImB + y*Ix + x) = 1;
			}
		}
	}

	//Release Mem
	if (IX_pix != NULL) free(IX_pix);
	if (IX_gray != NULL) free(IX_gray);
	if (IX_graygray != NULL) free(IX_graygray);
	if (IX_gray1 != NULL) free(IX_gray1);
	if (IX_graygray1 != NULL) free(IX_graygray1);
	if (IX_pix1 != NULL)free(IX_pix1);

	Mat outImg = Mat::zeros(InputImg.size(), CV_8UC1);
	for (int y = 0; y < Iy; y++)
	{
		uchar* v = outImg.ptr<uchar>(y);
		for (int x = 0; x < Ix; x++)
		{
			v[x] = *(outputImB + y*Ix + x);
		}
	}
	if (outputImB != nullptr) free(outputImB);
	if (inputIm != nullptr) free(inputIm);
	return outImg;
}









