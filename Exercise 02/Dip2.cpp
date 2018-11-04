//============================================================================
// Name        : Dip2.cpp
// Author      : Ronny Haensch
// Version     : 2.0
// Copyright   : -
// Description :
//============================================================================

#include "Dip2.h"

// convolution in spatial domain
/*
src:     input image
kernel:  filter kernel
return:  convolution result
*/
Mat Dip2::spatialConvolution(Mat &src, Mat &kernel)
{
	int kSize = kernel.rows;
	int r = (kSize - 1) / 2;
	Mat final(src.rows, src.cols, CV_32FC1);
	Mat dst(src.rows + kSize - 1, src.cols + kSize - 1, CV_32FC1);
	copyMakeBorder(src, dst, r, r, r, r, BORDER_CONSTANT, 0);

	int rows = final.rows;
	int cols = final.cols;
	int i, j, m, n;
	float temp;

	for (i = 0; i < rows; i++)
	{
		float *final_data = final.ptr<float>(i);
		for (j = 0; j < cols; j++)
		{
			temp = 0;
			for (m = 0; m < kSize; m++)
			{
				const float *data = dst.ptr<float>(i + m);
				data += j;
				const float *kernel_data = kernel.ptr<float>(m);
				for (n = 0; n < kSize; n++)
				{
					temp += (*data) * (*kernel_data);
					data++;
					kernel_data++;
				}
			}
			*final_data = temp;
			final_data++;
		}
	}

	return final;
}

// the average filter
// HINT: you might want to use Dip2::spatialConvolution(...) within this function
/*
src:     input image
kSize:   window size used by local average
return:  filtered image
*/
Mat Dip2::averageFilter(Mat &src, int kSize)
{
	float weight = (float)1 / (kSize * kSize);
	Mat kernel(kSize, kSize, CV_32FC1, Scalar::all(weight));
	return spatialConvolution(src, kernel);
}

// the median filter
/*
src:     input image
kSize:   window size used by median operation
return:  filtered image
*/
Mat Dip2::medianFilter(Mat &src, int kSize)
{
	// TO DO !!
	return src.clone();
}

// the bilateral filter
/*
src:     input image
kSize:   size of the kernel --> used to compute std-dev of spatial kernel
sigma:   standard-deviation of the radiometric kernel
return:  filtered image
*/
Mat Dip2::bilateralFilter(Mat &src, int kSize, double sigma)
{

	return src.clone();
}

// the non-local means filter
/*
src:   		input image
searchSize: size of search region
sigma: 		Optional parameter for weighting function
return:  	filtered image
*/
Mat Dip2::nlmFilter(Mat &src, int searchSize, double sigma)
{

	return src.clone();
}

/* *****************************
  GIVEN FUNCTIONS
***************************** */

// function loads input image, calls processing function, and saves result
void Dip2::run(void)
{

	// load images as grayscale
	cout << "load images" << endl;
	Mat noise1 = imread("noiseType_1.jpg", 0);
	if (!noise1.data)
	{
		cout << "noiseType_1.jpg not found" << endl;
		cout << "Press enter to exit" << endl;
		cin.get();
		exit(-3);
	}
	noise1.convertTo(noise1, CV_32FC1);
	Mat noise2 = imread("noiseType_2.jpg", 0);
	if (!noise2.data)
	{
		cout << "noiseType_2.jpg not found" << endl;
		cout << "Press enter to exit" << endl;
		cin.get();
		exit(-3);
	}
	noise2.convertTo(noise2, CV_32FC1);
	cout << "done" << endl;

	// apply noise reduction
	// TO DO !!!
	// ==> Choose appropriate noise reduction technique with appropriate parameters
	// ==> "average" or "median"? Why?
	// ==> try also "bilateral" (and if implemented "nlm")
	cout << "reduce noise" << endl;
	Mat restorated1 = noiseReduction(noise1, "average", 7);
	Mat restorated2 = noiseReduction(noise2, "average", 7);
	cout << "done" << endl;

	// save images
	cout << "save results" << endl;
	imwrite("restorated1 - average.jpg", restorated1);
	imwrite("restorated2 - average.jpg", restorated2);
	cout << "done" << endl;
}

// noise reduction
/*
src:     input image
method:  name of noise reduction method that shall be performed
	     "average" ==> moving average
         "median" ==> median filter
         "bilateral" ==> bilateral filter
         "nlm" ==> non-local means filter
kSize:   (spatial) kernel size
param:   if method == "bilateral", standard-deviation of radiometric kernel; if method == "nlm", (optional) parameter for similarity function
         can be ignored otherwise (default value = 0)
return:  output image
*/
Mat Dip2::noiseReduction(Mat &src, string method, int kSize, double param)
{

	// apply moving average filter
	if (method.compare("average") == 0)
	{
		return averageFilter(src, kSize);
	}
	// apply median filter
	if (method.compare("median") == 0)
	{
		return medianFilter(src, kSize);
	}
	// apply bilateral filter
	if (method.compare("bilateral") == 0)
	{
		return bilateralFilter(src, kSize, param);
	}
	// apply adaptive average filter
	if (method.compare("nlm") == 0)
	{
		return nlmFilter(src, kSize, param);
	}

	// if none of above, throw warning and return copy of original
	cout << "WARNING: Unknown filtering method! Returning original" << endl;
	cout << "Press enter to continue" << endl;
	cin.get();
	return src.clone();
}

// generates and saves different noisy versions of input image
/*
fname:   path to the input image
*/
void Dip2::generateNoisyImages(string fname)
{

	// load image, force gray-scale
	cout << "load original image" << endl;
	Mat img = imread(fname, 0);
	if (!img.data)
	{
		cout << "ERROR: file " << fname << " not found" << endl;
		cout << "Press enter to exit" << endl;
		cin.get();
		exit(-3);
	}

	// convert to floating point precision
	img.convertTo(img, CV_32FC1);
	cout << "done" << endl;

	// save original
	imwrite("original.jpg", img);

	// generate images with different types of noise
	cout << "generate noisy images" << endl;

	// some temporary images
	Mat tmp1(img.rows, img.cols, CV_32FC1);
	Mat tmp2(img.rows, img.cols, CV_32FC1);
	// first noise operation - Shot Noise
	float noiseLevel = 0.15;
	randu(tmp1, 0, 1);
	threshold(tmp1, tmp2, noiseLevel, 1, CV_THRESH_BINARY);
	multiply(tmp2, img, tmp2);
	threshold(tmp1, tmp1, 1 - noiseLevel, 1, CV_THRESH_BINARY);
	tmp1 *= 255;
	tmp1 = tmp2 + tmp1;
	threshold(tmp1, tmp1, 255, 255, CV_THRESH_TRUNC);
	// save image
	imwrite("noiseType_1.jpg", tmp1);

	// second noise operation - Gaussian Noise
	noiseLevel = 50;
	randn(tmp1, 0, noiseLevel);
	tmp1 = img + tmp1;
	threshold(tmp1, tmp1, 255, 255, CV_THRESH_TRUNC);
	threshold(tmp1, tmp1, 0, 0, CV_THRESH_TOZERO);
	// save image
	imwrite("noiseType_2.jpg", tmp1);

	cout << "done" << endl;
	cout << "Please run now: dip2 restorate" << endl;
}

// function calls some basic testing routines to test individual functions for correctness
void Dip2::test(void)
{

	test_spatialConvolution();
	test_averageFilter();
	test_medianFilter();

	cout << "Press enter to continue" << endl;
	cin.get();
}

// checks basic properties of the convolution result
void Dip2::test_spatialConvolution(void)
{

	Mat input = Mat::ones(9, 9, CV_32FC1);
	input.at<float>(4, 4) = 255;
	Mat kernel = Mat(3, 3, CV_32FC1, 1. / 9.);

	Mat output = spatialConvolution(input, kernel);

	if ((input.cols != output.cols) || (input.rows != output.rows))
	{
		cout << "ERROR: Dip2::spatialConvolution(): input.size != output.size --> Wrong border handling?" << endl;
		return;
	}
	if ((sum(output.row(0) < 0).val[0] > 0) ||
		(sum(output.row(0) > 255).val[0] > 0) ||
		(sum(output.row(8) < 0).val[0] > 0) ||
		(sum(output.row(8) > 255).val[0] > 0) ||
		(sum(output.col(0) < 0).val[0] > 0) ||
		(sum(output.col(0) > 255).val[0] > 0) ||
		(sum(output.col(8) < 0).val[0] > 0) ||
		(sum(output.col(8) > 255).val[0] > 0))
	{
		cout << "ERROR: Dip2::spatialConvolution(): Border of convolution result contains too large/small values --> Wrong border handling?" << endl;
		return;
	}
	else
	{
		if ((sum(output < 0).val[0] > 0) ||
			(sum(output > 255).val[0] > 0))
		{
			cout << "ERROR: Dip2::spatialConvolution(): Convolution result contains too large/small values!" << endl;
			return;
		}
	}
	float ref[9][9] = {{0, 0, 0, 0, 0, 0, 0, 0, 0},
					   {0, 1, 1, 1, 1, 1, 1, 1, 0},
					   {0, 1, 1, 1, 1, 1, 1, 1, 0},
					   {0, 1, 1, (8 + 255) / 9., (8 + 255) / 9., (8 + 255) / 9., 1, 1, 0},
					   {0, 1, 1, (8 + 255) / 9., (8 + 255) / 9., (8 + 255) / 9., 1, 1, 0},
					   {0, 1, 1, (8 + 255) / 9., (8 + 255) / 9., (8 + 255) / 9., 1, 1, 0},
					   {0, 1, 1, 1, 1, 1, 1, 1, 0},
					   {0, 1, 1, 1, 1, 1, 1, 1, 0},
					   {0, 0, 0, 0, 0, 0, 0, 0, 0}};
	for (int y = 1; y < 8; y++)
	{
		for (int x = 1; x < 8; x++)
		{
			if (abs(output.at<float>(y, x) - ref[y][x]) > 0.0001)
			{
				cout << "ERROR: Dip2::spatialConvolution(): Convolution result contains wrong values!" << endl;
				return;
			}
		}
	}
	input.setTo(0);
	input.at<float>(4, 4) = 255;
	kernel.setTo(0);
	kernel.at<float>(0, 0) = -1;
	output = spatialConvolution(input, kernel);
	if (abs(output.at<float>(5, 5) + 255.) < 0.0001)
	{
		cout << "ERROR: Dip2::spatialConvolution(): Is filter kernel \"flipped\" during convolution? (Check lecture/exercise slides)" << endl;
		return;
	}
	if ((abs(output.at<float>(2, 2) + 255.) < 0.0001) || (abs(output.at<float>(4, 4) + 255.) < 0.0001))
	{
		cout << "ERROR: Dip2::spatialConvolution(): Is anchor point of convolution the centre of the filter kernel? (Check lecture/exercise slides)" << endl;
		return;
	}
	cout << "Message: Dip2::spatialConvolution() seems to be correct" << endl;
}

// checks basic properties of the filtering result
void Dip2::test_averageFilter(void)
{

	Mat input = Mat::ones(9, 9, CV_32FC1);
	input.at<float>(4, 4) = 255;

	Mat output = averageFilter(input, 3);

	if ((input.cols != output.cols) || (input.rows != output.rows))
	{
		cout << "ERROR: Dip2::averageFilter(): input.size != output.size --> Wrong border handling?" << endl;
		return;
	}
	if ((sum(output.row(0) < 0).val[0] > 0) ||
		(sum(output.row(0) > 255).val[0] > 0) ||
		(sum(output.row(8) < 0).val[0] > 0) ||
		(sum(output.row(8) > 255).val[0] > 0) ||
		(sum(output.col(0) < 0).val[0] > 0) ||
		(sum(output.col(0) > 255).val[0] > 0) ||
		(sum(output.col(8) < 0).val[0] > 0) ||
		(sum(output.col(8) > 255).val[0] > 0))
	{
		cout << "ERROR: Dip2::averageFilter(): Border of result contains too large/small values --> Wrong border handling?" << endl;
		return;
	}
	else
	{
		if ((sum(output < 0).val[0] > 0) ||
			(sum(output > 255).val[0] > 0))
		{
			cout << "ERROR: Dip2::averageFilter(): Result contains too large/small values!" << endl;
			return;
		}
	}
	float ref[9][9] = {{0, 0, 0, 0, 0, 0, 0, 0, 0},
					   {0, 1, 1, 1, 1, 1, 1, 1, 0},
					   {0, 1, 1, 1, 1, 1, 1, 1, 0},
					   {0, 1, 1, (8 + 255) / 9., (8 + 255) / 9., (8 + 255) / 9., 1, 1, 0},
					   {0, 1, 1, (8 + 255) / 9., (8 + 255) / 9., (8 + 255) / 9., 1, 1, 0},
					   {0, 1, 1, (8 + 255) / 9., (8 + 255) / 9., (8 + 255) / 9., 1, 1, 0},
					   {0, 1, 1, 1, 1, 1, 1, 1, 0},
					   {0, 1, 1, 1, 1, 1, 1, 1, 0},
					   {0, 0, 0, 0, 0, 0, 0, 0, 0}};
	for (int y = 1; y < 8; y++)
	{
		for (int x = 1; x < 8; x++)
		{
			if (abs(output.at<float>(y, x) - ref[y][x]) > 0.0001)
			{
				cout << "ERROR: Dip2::averageFilter(): Result contains wrong values!" << endl;
				return;
			}
		}
	}
	cout << "Message: Dip2::averageFilter() seems to be correct" << endl;
}

// checks basic properties of the filtering result
void Dip2::test_medianFilter(void)
{

	Mat input = Mat::ones(9, 9, CV_32FC1);
	input.at<float>(4, 4) = 255;

	Mat output = medianFilter(input, 3);

	if ((input.cols != output.cols) || (input.rows != output.rows))
	{
		cout << "ERROR: Dip2::medianFilter(): input.size != output.size --> Wrong border handling?" << endl;
		return;
	}
	if ((sum(output.row(0) < 0).val[0] > 0) ||
		(sum(output.row(0) > 255).val[0] > 0) ||
		(sum(output.row(8) < 0).val[0] > 0) ||
		(sum(output.row(8) > 255).val[0] > 0) ||
		(sum(output.col(0) < 0).val[0] > 0) ||
		(sum(output.col(0) > 255).val[0] > 0) ||
		(sum(output.col(8) < 0).val[0] > 0) ||
		(sum(output.col(8) > 255).val[0] > 0))
	{
		cout << "ERROR: Dip2::medianFilter(): Border of result contains too large/small values --> Wrong border handling?" << endl;
		return;
	}
	else
	{
		if ((sum(output < 0).val[0] > 0) ||
			(sum(output > 255).val[0] > 0))
		{
			cout << "ERROR: Dip2::medianFilter(): Result contains too large/small values!" << endl;
			return;
		}
	}
	for (int y = 1; y < 8; y++)
	{
		for (int x = 1; x < 8; x++)
		{
			if (abs(output.at<float>(y, x) - 1.) > 0.0001)
			{
				cout << "ERROR: Dip2::medianFilter(): Result contains wrong values!" << endl;
				return;
			}
		}
	}
	cout << "Message: Dip2::medianFilter() seems to be correct" << endl;
}
