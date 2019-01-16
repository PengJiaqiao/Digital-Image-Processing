//============================================================================
// Name        : Dip5.cpp
// Author      : Ronny Haensch
// Version     : 2.0
// Copyright   : -
// Description :
//============================================================================

#include "Dip5.h"

// uses structure tensor to define interest points (foerstner)
void Dip5::getInterestPoints(const Mat &img, double sigma, vector<KeyPoint> &points)
{
	// TO DO !!!
	Mat Gx, Gy, Gx_sq, Gy_sq, Gx_Gy;
	Mat x_kernel = createFstDevKernel(sigma);
	Mat y_kernel = x_kernel.clone();
	y_kernel = y_kernel.t();

	//Gx, Gy, Gx_sq, Gy_sq, Gx_Gy
	filter2D(img, Gx, -1, x_kernel);
	filter2D(img, Gy, -1, y_kernel);

	Gx_sq = Gx.mul(Gx);
	Gy_sq = Gy.mul(Gy);
	Gx_Gy = Gx.mul(Gy);

	//Average with Gaussian
	GaussianBlur(Gx_sq, Gx_sq, Size(5, 5), 0, 0);
	GaussianBlur(Gy_sq, Gy_sq, Size(5, 5), 0, 0);
	GaussianBlur(Gx_Gy, Gx_Gy, Size(5, 5), 0, 0);

	//trace & det of structure tensor
	Mat trace = Gx_sq + Gy_sq;
	Mat det = Gx_sq.mul(Gy_sq) - Gx_Gy.mul(Gx_Gy);

	//weight
	Mat weight;
	divide(det, trace, weight);

	//weight non-max supression
	weight = nonMaxSuppression(weight);
	Scalar w_mean = mean(weight);
	int w_min = w_mean.val[0] * 0.5; //changeable

	//weight threshold
	threshold(weight * -1, weight, w_min * -1, 0, THRESH_TRUNC);
	weight *= -1;

	//isotropy
	Mat q;
	divide(4 * det, trace.mul(trace), q);

	//isotropy non-max supression
	q = nonMaxSuppression(q);
	float q_min = 0.5; //changeable

	//isotropy threshold
	threshold(q * -1, q, q_min * -1, 0, THRESH_TRUNC);
	q *= -1;

	//keypoints
	for (int i = 0; i < img.rows; i++)
	{
		float *weight_data = weight.ptr<float>(i);
		float *q_data = q.ptr<float>(i);
		for (int j = 0; j < img.cols; j++)
		{
			if (*weight_data > w_min && *q_data > q_min)
			{
				KeyPoint point = KeyPoint(j, i, 1);
				points.push_back(point);
			}
			weight_data++;
			q_data++;
		}
	}
}

// creates kernel representing fst derivative of a Gaussian kernel in x-direction
/*
sigma	standard deviation of the Gaussian kernel
return	the calculated kernel
*/
Mat Dip5::createFstDevKernel(double sigma)
{
	// TO DO !!!
	int r = (int)(3 * sigma);
	int kernel_size = 2 * r + 1; // radius is 3*sigma empirical
	Mat kernel(kernel_size, kernel_size, CV_32FC1);
	float sigma_2 = (float)pow(sigma, 2);
	float sigma_4 = (float)pow(sigma, 4);

	for (int i = 0; i < kernel_size; i++)
	{
		float *dst = kernel.ptr<float>(i);
		int y_dist = i - r;
		for (int j = 0; j < kernel_size; j++)
		{
			int x_dist = j - r;
			*dst++ = -x_dist * exp(-(pow(x_dist, 2) + pow(y_dist, 2)) / (2 * sigma_2)) / (2 * CV_PI * sigma_4);
		}
	}
	return kernel;
}

/* *****************************
  GIVEN FUNCTIONS
***************************** */

// function calls processing function
/*
in		:  input image
points	:	detected keypoints
*/
void Dip5::run(const Mat &in, vector<KeyPoint> &points)
{
	this->getInterestPoints(in, this->sigma, points);
}

// non-maxima suppression
// if any of the pixel at the 4-neighborhood is greater than current pixel, set it to zero
Mat Dip5::nonMaxSuppression(const Mat &img)
{

	Mat out = img.clone();

	for (int x = 1; x < out.cols - 1; x++)
	{
		for (int y = 1; y < out.rows - 1; y++)
		{
			if (img.at<float>(y - 1, x) >= img.at<float>(y, x))
			{
				out.at<float>(y, x) = 0;
				continue;
			}
			if (img.at<float>(y, x - 1) >= img.at<float>(y, x))
			{
				out.at<float>(y, x) = 0;
				continue;
			}
			if (img.at<float>(y, x + 1) >= img.at<float>(y, x))
			{
				out.at<float>(y, x) = 0;
				continue;
			}
			if (img.at<float>(y + 1, x) >= img.at<float>(y, x))
			{
				out.at<float>(y, x) = 0;
				continue;
			}
		}
	}
	return out;
}

// Function displays image (after proper normalization)
/*
win   :  Window name
img   :  Image that shall be displayed
cut   :  whether to cut or scale values outside of [0,255] range
*/
void Dip5::showImage(const Mat &img, const char *win, int wait, bool show, bool save)
{

	Mat aux = img.clone();

	// scale and convert
	if (img.channels() == 1)
		normalize(aux, aux, 0, 255, CV_MINMAX);
	aux.convertTo(aux, CV_8UC1);
	// show
	if (show)
	{
		imshow(win, aux);
		waitKey(wait);
	}
	// save
	if (save)
		imwrite((string(win) + string(".png")).c_str(), aux);
}
