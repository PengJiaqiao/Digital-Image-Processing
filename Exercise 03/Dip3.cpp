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
	Mat GaussianKernel(kSize, kSize, CV_32FC1);
	//Compute two 1-dimensional Gaussian Filter and use matrix multiplication to get 2-dimensional Gaussian Filter
	Mat x(1, kSize, CV_32FC1);
	Mat y(1, kSize, CV_32FC1);
	const float sigma = (float)kSize / 5;
	const float _sigma = sigma * sigma;
	float temp, sum = 0; //sum for Normalizing

	float *dst = x.ptr<float>(0);
	float *_dst = dst + kSize - 1;
	for (int i = 0; i < r; i++)
	{
		temp = std::exp(-0.5 * (std::pow(r - i, 2)) / _sigma);
		*dst++ = temp;
		*_dst-- = temp;
		sum += temp;
	}
	x.at<float>(0, r) = 1;
	sum = sum * 2 + 1;
	x /= sum;

	y = x.clone();
	GaussianKernel = y.t() * x;

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
	/*Cut source image into 4 parts (if dx > 0 & dy > 0)
	0,0-----------------------------cols-1,0
	-			      -			     	-
	-		 0	      -		   1     	-
    -				  -			     	-
	-				  -			     	-
	----------cols-dx,rows-dy(in 3)-----cols-1,rows-dy
	-				  -			     	-
	-		 2		  -		   3     	-
	-				  -			     	-
	-				  -			     	-	
	----------cols-dx,rows-1------cols-1,rows-1

	Cut result image into 4 parts
	0,0------------dx-1,0----------cols-1,0
	-			      -			     	-
	-		 3		  -		   2     	-
    -				  -			     	-
	-				  -			     	-
	0,dy-1----dx-1,dy-1(in 3)------------
	-				  -			     	-
	-		 1		  -		   0     	-
	-				  -			     	-
	-				  -			     	-	
	-------------------------------------*/
	Mat dst(in.size(), CV_32FC1);
	int x, y;
	// Adjust parameters
	if (dx >= 0)
		x = dx % in.cols;
	else
		x = dx % in.cols + in.cols;
	if (dy >= 0)
		y = dy % in.rows;
	else
		y = dy % in.rows + in.rows;

	Mat tmp0, tmp1, tmp2, tmp3;
	Mat part0(in, Rect(0, 0, in.cols - x, in.rows - y));
	Mat part1(in, Rect(in.cols - x, 0, x, in.rows - y));
	Mat part2(in, Rect(0, in.rows - y, in.cols - x, y));
	Mat part3(in, Rect(in.cols - x, in.rows - y, x, y));
	part0.copyTo(tmp0);
	part1.copyTo(tmp1);
	part2.copyTo(tmp2);
	part3.copyTo(tmp3);
	tmp0.copyTo(in(Rect(x, y, in.cols - x, in.rows - y)));
	tmp1.copyTo(in(Rect(0, y, x, in.rows - y)));
	tmp2.copyTo(in(Rect(x, 0, in.cols - x, y)));
	tmp3.copyTo(in(Rect(0, 0, x, y)));

	return in;
}

//Performes convolution by multiplication in frequency domain
/*
in       input image
kernel   filter kernel
return   output image
*/
Mat Dip3::frequencyConvolution(const Mat &in, const Mat &kernel)
{
	// Generate padding kernel with the same size of origin image
	Mat kernel_padding(in.size(), CV_32FC1);
	int r_top, r_bottom, r_left, r_right;
	r_top = (in.rows - kernel.rows) / 2;
	r_bottom = in.rows - kernel.rows - r_top;
	r_left = (in.cols - kernel.cols) / 2;
	r_right = in.cols - kernel.cols - r_left;
	copyMakeBorder(kernel, kernel_padding, r_top, r_bottom, r_left, r_right, BORDER_CONSTANT, 0);

	// Centre the kernel
	kernel_padding = circShift(kernel_padding, in.cols - in.cols / 2, in.rows - in.rows / 2);
	Mat kernel_planes[] = {Mat_<float>(kernel_padding), Mat::zeros(kernel_padding.size(), CV_32F)};
	Mat kernel_complex;
	merge(kernel_planes, 2, kernel_complex);

	Mat in_planes[] = {Mat_<float>(in), Mat::zeros(in.size(), CV_32F)};
	Mat in_complex;
	merge(in_planes, 2, in_complex);
	in_complex /= in.cols * in.rows;

	dft(in_complex, in_complex, DFT_COMPLEX_OUTPUT);
	dft(kernel_complex, kernel_complex, DFT_COMPLEX_OUTPUT);
	Mat dst;
	mulSpectrums(in_complex, kernel_complex, dst, 0);
	dft(dst, dst, DFT_INVERSE | DFT_REAL_OUTPUT);

	return dst;
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
	Mat tmp(in.size(), CV_32FC1);

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
	// For test use only
	/*if (sum(tmp > 255).val[0] > 0)
	{
		cout << "ERROR: Dip3::frequencyConvolution(): Convolution result contains too large values!" << endl;
	}

	switch (type)
	{
	case 0:
		imwrite("spatialDomain.jpg", tmp);
		break;
	case 1:
		imwrite("frequencyDomain.jpg", tmp);
		break;
	case 2:
		imwrite("sperableFilters.jpg", tmp);
		break;
	case 3:
		imwrite("integralImage.jpg", tmp);
		break;
	}*/

	Mat edge = in - tmp;
	threshold(edge, edge, thresh, 0, THRESH_TOZERO);
	edge *= scale;
	in += edge;

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
	Mat dst(src.size(), CV_32FC1);
	Mat kernel_flipped(kernel.size(), CV_32FC1);
	Mat src_padding(src.rows + kernel.rows - 1, src.cols + kernel.cols - 1, CV_32FC1);
	copyMakeBorder(src, src_padding, kernel.rows / 2, kernel.rows / 2, kernel.cols / 2, kernel.cols / 2, BORDER_REPLICATE);

	int i, j, m, n;
	float temp;
	// Generate a flipped kernel first
	for (i = 0; i < kernel.rows; i++)
	{
		float *kernel_data = kernel_flipped.ptr<float>(i);
		const float *kernel_origin = kernel.ptr<float>(kernel.rows - i - 1);
		kernel_origin += kernel.cols - 1;
		for (j = 0; j < kernel.cols; j++)
		{
			*kernel_data++ = *kernel_origin--; //Coordinates flipped
		}
	}

	for (i = 0; i < dst.rows; i++)
	{
		float *dst_data = dst.ptr<float>(i);
		for (j = 0; j < dst.cols; j++)
		{
			temp = 0;
			for (m = 0; m < kernel.rows; m++)
			{
				const float *src_data = src_padding.ptr<float>(i + m);
				src_data += j;
				const float *kernel_data = kernel_flipped.ptr<float>(m);
				for (n = 0; n < kernel.cols; n++)
				{
					temp += (*src_data++) * (*kernel_data++);
				}
			}
			*dst_data++ = temp;
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
	int r = size / 2;
	Mat filter(1, size, CV_32FC1); // one-dimensional Gaussian Filter
	const float sigma = (float)size / 5;
	const float _sigma = sigma * sigma;
	float temp, sum = 0; //sum for Normalizing

	float *dst = filter.ptr<float>(0);
	float *_dst = dst + size - 1;
	for (int i = 0; i < r; i++)
	{
		temp = std::exp(-0.5 * (std::pow(r - i, 2)) / _sigma);
		*dst++ = temp;
		*_dst-- = temp;
		sum += temp;
	}
	filter.at<float>(0, r) = 1;
	sum = sum * 2 + 1;
	filter /= sum;

	return spatialConvolution(spatialConvolution(src, filter.t()), filter);
}

// convolution in spatial domain by integral images
/*
src:    input image
size     size of filter kernel
return:  convolution result
*/
Mat Dip3::satFilter(const Mat &src, int size)
{
	int r = size / 2;
	int size_square = size * size;
	Mat dst(src.size(), CV_32FC1);
	Mat src_padding(src.rows + 2 * r, src.cols + 2 * r, CV_32FC1);
	copyMakeBorder(src, src_padding, r, r, r, r, BORDER_REPLICATE); // One more row in the top and one more column in the left for integral image padding
	Mat Integral(src.rows + 1, src.cols + 1, CV_32FC1);
	integral(src_padding, Integral, CV_32FC1);

	int i, j;
	for (i = 0; i < dst.rows; i++)
	{
		float *dst_data = dst.ptr<float>(i);
		for (j = 0; j < dst.cols; j++)
		{
			*dst_data++ = (Integral.at<float>(i + size, j + size) - Integral.at<float>(i + size, j) - Integral.at<float>(i, j + size) + Integral.at<float>(i, j)) / size_square;
		}
	}

	return dst;
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
	//I am wondering whether my teammates will check this code carefully
	//Please DELETE YOUR NAME ONLY
	//Fayanjuola, Ayotomiwa Augustus
	//Long, Zhou
	//Zhang, Liting
	return usm(in, smoothType, size, thresh, scale);
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
