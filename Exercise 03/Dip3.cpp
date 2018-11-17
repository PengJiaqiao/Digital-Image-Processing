//============================================================================
// Name    : Dip3.cpp
// Author   : Ronny Haensch
// Version    : 2.0
// Copyright   : -
// Description :
//============================================================================

#include "Dip3.h"

// Generates gaussian filter kernel of given size
/*
kSize:     kernel size (used to calculate standard deviation)
return:    the generated filter kernel
*/
Mat Dip3::createGaussianKernel(int kSize)
{
	int r = kSize / 2;
	int i, j;
	Mat GaussianKernel(kSize, kSize, CV_32FC1);
	//Compute two 1-dimensional Gaussian Filter and use matrix multiplication to get 2-dimensional Gaussian Filter
	Mat x(1, kSize, CV_32FC1);
	Mat y(1, kSize, CV_32FC1);
	const float pi = 3.1415926;
	const float sigma = (float)kSize / 5;
	const float _sigma = sigma * sigma;
	float temp, sum = 0; //sum for Normalizing

	float *dst = x.ptr<float>(0);
	float *_dst = dst;
	_dst += kSize - 1;
	for (i = 0; i < r; i++)
	{
		temp = std::exp(-0.5 * (std::pow(r - i, 2)) / _sigma);
		*dst++ = temp;
		*_dst-- = temp;
	}
	x.at<float>(0, r) = 1;

	y = x.clone();
	GaussianKernel = y.t() * x;
	GaussianKernel *= 1 / (2 * pi * _sigma);

	for (i = 0; i < r; i++) //Normalizing
	{
		const float *dst = GaussianKernel.ptr<float>(i);
		for (j = 0; j < kSize; j++)
		{
			sum += *dst++;
		}
	}
	GaussianKernel /= sum;

	return GaussianKernel;
}

// Performes a circular shift in (dx,dy) direction
/*
in       input matrix
dx       shift in x-direction
dy       shift in y-direction
return   circular shifted matrix
*/
Mat Dip3::circShift(const Mat &in, int dx, int dy)
{
	Mat dst(in.rows, in.cols, CV_32FC1);
	int i, j;
	/*Cut source image into 4 parts
	0,0-----------------------------cols-1,0
	-			      -			     	-
	-				  -			     	-
    -				  -			     	-
	-				  -			     	-
	----------cols-dx,rows-dy-----cols-1,rows-dy
	-				  -			     	-
	-				  -			     	-
	-				  -			     	-
	-				  -			     	-	
	----------cols-dx,rows-1------cols-1,rows-1

	Cut result image into 4 parts
	0,0------------dx-1,0----------cols-1,0
	-			      -			     	-
	-				  -			     	-
    -				  -			     	-
	-				  -			     	-
	0,dy-1--------dx-1,dy-1--------------
	-				  -			     	-
	-				  -			     	-
	-				  -			     	-
	-				  -			     	-	
	-------------------------------------*/
	for (i = 0; i < dy; i++)
	{
		const float *in_data = in.ptr<float>(in.rows - dy + i);
		in_data += in.cols - dx;
		float *dst_data = dst.ptr<float>(i);
		for (j = 0; j < dx; j++)
		{
			*dst_data++ = *in_data++;
		}
	}
	for (i = 0; i < dy; i++)
	{
		const float *in_data = in.ptr<float>(in.rows - dy + i);
		float *dst_data = dst.ptr<float>(i);
		dst_data += dx;
		for (j = dx; j < in.cols; j++)
		{
			*dst_data++ = *in_data++;
		}
	}
	for (i = dy; i < in.rows; i++)
	{
		const float *in_data = in.ptr<float>(i - dy);
		in_data += in.cols - dx;
		float *dst_data = dst.ptr<float>(i);
		for (j = 0; j < dy; j++)
		{
			*dst_data++ = *in_data++;
		}
	}
	for (i = dy; i < in.rows; i++)
	{
		const float *in_data = in.ptr<float>(i - dy);
		float *dst_data = dst.ptr<float>(i);
		dst_data += dx;
		for (j = dx; j < in.cols; j++)
		{
			*dst_data++ = *in_data++;
		}
	}
	return dst;
}

//Performes convolution by multiplication in frequency domain
/*
in       input image
kernel   filter kernel
return   output image
*/
Mat Dip3::frequencyConvolution(const Mat &in, const Mat &kernel)
{

	// TO DO !!!

	return in;
}

// Performs UnSharp Masking to enhance fine image structures
/*
in       the input image
type     integer defining how convolution for smoothing operation is done
         0 <==> spatial domain; 1 <==> frequency domain; 2 <==> seperable filter; 3 <==> integral image
size     size of used smoothing kernel
thresh   minimal intensity difference to perform operation
scale    scaling of edge enhancement
return   enhanced image
*/
Mat Dip3::usm(const Mat &in, int type, int size, double thresh, double scale)
{

	// some temporary images
	Mat tmp(in.rows, in.cols, CV_32FC1);

	// calculate edge enhancement

	// 1: smooth original image
	//    save result in tmp for subsequent usage
	switch (type)
	{
	case 0:
		tmp = mySmooth(in, size, 0);
		break;
	case 1:
		tmp = mySmooth(in, size, 1);
		break;
	case 2:
		tmp = mySmooth(in, size, 2);
		break;
	case 3:
		tmp = mySmooth(in, size, 3);
		break;
	default:
		GaussianBlur(in, tmp, Size(floor(size / 2) * 2 + 1, floor(size / 2) * 2 + 1), size / 5., size / 5.);
	}

	// TO DO !!!

	return in;
}

// convolution in spatial domain
/*
src:    input image
kernel:  filter kernel
return:  convolution result
*/
Mat Dip3::spatialConvolution(const Mat &src, const Mat &kernel)
{
	int kSize = kernel.rows;
	int r = (kSize - 1) / 2;
	int rows = src.rows;
	int cols = src.cols;
	Mat dst(rows, cols, CV_32FC1);
	Mat src_padding(rows + kSize - 1, cols + kSize - 1, CV_32FC1);
	Mat kernel_flipped(kernel.rows, kernel.cols, CV_32FC1);
	copyMakeBorder(src, src_padding, r, r, r, r, BORDER_REPLICATE);

	int i, j;
	int _r = 2 * r;
	for (i = 0; i < kernel.rows; i++)
	{
		float *kf_data = kernel_flipped.ptr<float>(i);
		for (j = 0; j < kernel.cols; j++)
		{
			*kf_data++ = kernel.at<float>(_r - i, _r - j); //Coordinates flipped
		}
	}

	int m, n;
	float temp;
	for (i = 0; i < rows; i++)
	{
		float *dst_data = dst.ptr<float>(i);
		for (j = 0; j < cols; j++)
		{
			temp = 0;
			for (m = 0; m < kSize; m++)
			{
				const float *data = src_padding.ptr<float>(i + m);
				data += j;
				const float *kernel_flipped_data = kernel_flipped.ptr<float>(m);
				for (n = 0; n < kSize; n++)
				{
					temp += (*data) * (*kernel_flipped_data);
					data++;
					kernel_flipped_data++;
				}
			}
			*dst_data = temp;
			dst_data++;
		}
	}

	return dst;
}

// convolution in spatial domain by seperable filters
/*
src:    input image
size     size of filter kernel
return:  convolution result
*/
Mat Dip3::seperableFilter(const Mat &src, int size)
{

	// optional

	return src;
}

// convolution in spatial domain by integral images
/*
src:    input image
size     size of filter kernel
return:  convolution result
*/
Mat Dip3::satFilter(const Mat &src, int size)
{

	// optional

	return src;
}

/* *****************************
  GIVEN FUNCTIONS
***************************** */

// function calls processing function
/*
in       input image
type     integer defining how convolution for smoothing operation is done
         0 <==> spatial domain; 1 <==> frequency domain
size     size of used smoothing kernel
thresh   minimal intensity difference to perform operation
scale    scaling of edge enhancement
return   enhanced image
*/
Mat Dip3::run(const Mat &in, int smoothType, int size, double thresh, double scale)
{
	Mat img = imread("demo.jpg", 0);
	img.convertTo(img, CV_32FC1);
	imwrite("demo - gray.jpg",img);
	Mat dst = circShift(img, 150, 550);
	imwrite("demo - circular shift.jpg", dst);
}

// Performes smoothing operation by convolution
/*
in       input image
size     size of filter kernel
type     how is smoothing performed?
return   smoothed image
*/
Mat Dip3::mySmooth(const Mat &in, int size, int type)
{

	// create filter kernel
	Mat kernel = createGaussianKernel(size);

	// perform convoltion
	switch (type)
	{
	case 0:
		return spatialConvolution(in, kernel); // 2D spatial convolution
	case 1:
		return frequencyConvolution(in, kernel); // 2D convolution via multiplication in frequency domain
	case 2:
		return seperableFilter(in, size); // seperable filter
	case 3:
		return satFilter(in, size); // integral image
	default:
		return frequencyConvolution(in, kernel);
	}
}

// function calls basic testing routines to test individual functions for correctness
void Dip3::test(void)
{

	test_createGaussianKernel();
	test_circShift();
	test_frequencyConvolution();
	cout << "Press enter to continue" << endl;
	cin.get();
}

void Dip3::test_createGaussianKernel(void)
{

	Mat k = createGaussianKernel(11);

	if (abs(sum(k).val[0] - 1) > 0.0001)
	{
		cout << "ERROR: Dip3::createGaussianKernel(): Sum of all kernel elements is not one!" << endl;
		return;
	}
	if (sum(k >= k.at<float>(5, 5)).val[0] / 255 != 1)
	{
		cout << "ERROR: Dip3::createGaussianKernel(): Seems like kernel is not centered!" << endl;
		return;
	}
	cout << "Message: Dip3::createGaussianKernel() seems to be correct" << endl;
}

void Dip3::test_circShift(void)
{

	Mat in = Mat::zeros(3, 3, CV_32FC1);
	in.at<float>(0, 0) = 1;
	in.at<float>(0, 1) = 2;
	in.at<float>(1, 0) = 3;
	in.at<float>(1, 1) = 4;
	Mat ref = Mat::zeros(3, 3, CV_32FC1);
	ref.at<float>(0, 0) = 4;
	ref.at<float>(0, 2) = 3;
	ref.at<float>(2, 0) = 2;
	ref.at<float>(2, 2) = 1;

	if (sum((circShift(in, -1, -1) == ref)).val[0] / 255 != 9)
	{
		cout << "ERROR: Dip3::circShift(): Result of circshift seems to be wrong!" << endl;
		return;
	}
	cout << "Message: Dip3::circShift() seems to be correct" << endl;
}

void Dip3::test_frequencyConvolution(void)
{

	Mat input = Mat::ones(9, 9, CV_32FC1);
	input.at<float>(4, 4) = 255;
	Mat kernel = Mat(3, 3, CV_32FC1, 1. / 9.);

	Mat output = frequencyConvolution(input, kernel);

	if ((sum(output < 0).val[0] > 0) or (sum(output > 255).val[0] > 0))
	{
		cout << "ERROR: Dip3::frequencyConvolution(): Convolution result contains too large/small values!" << endl;
		return;
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
				cout << "ERROR: Dip3::frequencyConvolution(): Convolution result contains wrong values!" << endl;
				return;
			}
		}
	}
	cout << "Message: Dip3::frequencyConvolution() seems to be correct" << endl;
}
