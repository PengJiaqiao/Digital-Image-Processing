//============================================================================
// Name        : Dip4.cpp
// Author      : Ronny Haensch
// Version     : 2.0
// Copyright   : -
// Description :
//============================================================================

#include "Dip4.h"

// Performes a circular shift in (dx,dy) direction
/*
in       :  input matrix
dx       :  shift in x-direction
dy       :  shift in y-direction
return   :  circular shifted matrix
*/
Mat Dip4::circShift(const Mat &in, int dx, int dy)
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
	if (dx == 0 && dy == 0)
		return in;
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

// Function applies the inverse filter to restorate a degraded image
/*
degraded :  degraded input image
filter   :  filter which caused degradation
return   :  restorated output image
*/
Mat Dip4::inverseFilter(const Mat &degraded, const Mat &filter)
{
	//copy filter to larger template
	Mat big_filter = Mat::zeros(degraded.size(), CV_32FC1);
	Mat mask = big_filter(Rect(0, 0, filter.cols, filter.rows));
	filter.copyTo(mask);
	big_filter = circShift(big_filter.clone(), -1 * int(filter.cols / 2), -1 * int(filter.rows / 2));

	//pad zero to the second channel
	Mat big_filter_pad, img_pad;
	Mat pad = Mat::zeros(degraded.size(), CV_32FC1);
	vector<Mat> pad_(2);
	pad_[0] = big_filter.clone();
	pad_[1] = pad.clone();
	merge(pad_, big_filter_pad);

	vector<Mat> pad_img_(2);
	pad_img_[0] = degraded.clone();
	pad_img_[1] = pad.clone();
	merge(pad_img_, img_pad);
	img_pad /= degraded.rows * degraded.cols;

	//dft
	Mat dft_filter, dft_img;
	dft(big_filter_pad, dft_filter, DFT_COMPLEX_OUTPUT);
	dft(img_pad, dft_img, DFT_COMPLEX_OUTPUT);

	//conjugate Pk_star
	vector<Mat> split_(2);
	split(dft_filter, split_);
	split_[1] *= -1;

	//denominator
	Mat denom;
	denom = split_[0].mul(split_[0]) + split_[1].mul(split_[1]);

	//division
	Mat Q_complex;
	Mat Q_planes[2];
	divide(split_[0], denom, Q_planes[0]);
	divide(split_[1], denom, Q_planes[1]);

	//replace by thresholding
	float epsilon = 0.05;
	float epsilon_sqrt = epsilon * epsilon;
	float epsilon_ = 1 / epsilon;
	for (int i = 0; i < denom.rows; i++)
	{
		const float *denom_data = denom.ptr<float>(i);
		float *Q_planes_0 = Q_planes[0].ptr<float>(i);
		float *Q_planes_1 = Q_planes[1].ptr<float>(i);
		for (int j = 0; j < denom.cols; j++)
		{
			if (*denom_data < epsilon_sqrt)
			{
				*Q_planes_0 = epsilon_;
				*Q_planes_1 = 0;
			}
			denom_data++;
			Q_planes_0++;
			Q_planes_1++;
		}
	}
	merge(Q_planes, 2, Q_complex);

	//apply filter
	Mat dst;
	mulSpectrums(dft_img, Q_complex, dst, 0);
	dft(dst, dst, DFT_INVERSE | DFT_REAL_OUTPUT);

	return dst;
}

// Function applies the wiener filter to restorate a degraded image
/*
degraded :  degraded input image
filter   :  filter which caused degradation
snr      :  signal to noise ratio of the input image
return   :   restorated output image
*/
Mat Dip4::wienerFilter(const Mat &degraded, const Mat &filter, double snr)
{
	//copy filter to larger template
	Mat big_filter = Mat::zeros(degraded.size(), CV_32FC1);
	Mat mask = big_filter(Rect(0, 0, filter.cols, filter.rows));
	filter.copyTo(mask);
	big_filter = circShift(big_filter.clone(), -1 * int(filter.cols / 2), -1 * int(filter.rows / 2));

	//pad zero to the second channel
	Mat big_filter_pad, img_pad;
	Mat pad = Mat::zeros(degraded.size(), CV_32FC1);
	vector<Mat> pad_(2);
	pad_[0] = big_filter.clone();
	pad_[1] = pad.clone();
	merge(pad_, big_filter_pad);

	vector<Mat> pad_img_(2);
	pad_img_[0] = degraded.clone();
	pad_img_[1] = pad.clone();
	merge(pad_img_, img_pad);
	img_pad /= degraded.rows * degraded.cols;

	//dft
	Mat dft_filter, dft_img;
	dft(big_filter_pad, dft_filter, DFT_COMPLEX_OUTPUT);
	dft(img_pad, dft_img, DFT_COMPLEX_OUTPUT);

	//conjugate Pk_star
	vector<Mat> split_(2);
	split(dft_filter, split_);
	split_[1] *= -1;

	//denominator
	Mat denom;
	denom = split_[0].mul(split_[0]) + split_[1].mul(split_[1]);
	denom += 1 / snr / snr;

	//division
	Mat Q_complex;
	Mat Q_planes[2];
	divide(split_[0], denom, Q_planes[0]);
	divide(split_[1], denom, Q_planes[1]);
	merge(Q_planes, 2, Q_complex);

	//apply filter
	Mat dst;
	mulSpectrums(dft_img, Q_complex, dst, 0);
	dft(dst, dst, DFT_INVERSE | DFT_REAL_OUTPUT);

	return dst;
}

/* *****************************
  GIVEN FUNCTIONS
***************************** */

// function calls processing function
/*
in                   :  input image
restorationType     :  integer defining which restoration function is used
kernel               :  kernel used during restoration
snr                  :  signal-to-noise ratio (only used by wieder filter)
return               :  restorated image
*/
Mat Dip4::run(const Mat &in, string restorationType, const Mat &kernel, double snr)
{

	if (restorationType.compare("wiener") == 0)
	{
		return wienerFilter(in, kernel, snr);
	}
	else
	{
		return inverseFilter(in, kernel);
	}
}

// function degrades the given image with gaussian blur and additive gaussian noise
/*
img         :  input image
degradedImg :  degraded output image
filterDev   :  standard deviation of kernel for gaussian blur
snr         :  signal to noise ratio for additive gaussian noise
return      :  the used gaussian kernel
*/
Mat Dip4::degradeImage(const Mat &img, Mat &degradedImg, double filterDev, double snr)
{

	int kSize = round(filterDev * 3) * 2 - 1;

	Mat gaussKernel = getGaussianKernel(kSize, filterDev, CV_32FC1);
	gaussKernel = gaussKernel * gaussKernel.t();

	Mat imgs = img.clone();
	dft(imgs, imgs, CV_DXT_FORWARD, img.rows);
	Mat kernels = Mat::zeros(img.rows, img.cols, CV_32FC1);
	int dx, dy;
	dx = dy = (kSize - 1) / 2.;
	for (int i = 0; i < kSize; i++)
		for (int j = 0; j < kSize; j++)
			kernels.at<float>((i - dy + img.rows) % img.rows, (j - dx + img.cols) % img.cols) = gaussKernel.at<float>(i, j);
	dft(kernels, kernels, CV_DXT_FORWARD);
	mulSpectrums(imgs, kernels, imgs, 0);
	dft(imgs, degradedImg, CV_DXT_INV_SCALE, img.rows);

	Mat mean, stddev;
	meanStdDev(img, mean, stddev);

	Mat noise = Mat::zeros(img.rows, img.cols, CV_32FC1);
	randn(noise, 0, stddev.at<double>(0) / snr);
	degradedImg = degradedImg + noise;
	threshold(degradedImg, degradedImg, 255, 255, CV_THRESH_TRUNC);
	threshold(degradedImg, degradedImg, 0, 0, CV_THRESH_TOZERO);

	return gaussKernel;
}

// Function displays image (after proper normalization)
/*
win   :  Window name
img   :  image that shall be displayed
cut   :  determines whether to cut or scale values outside of [0,255] range
*/
void Dip4::showImage(const char *win, const Mat &img, bool cut)
{

	Mat tmp = img.clone();

	if (tmp.channels() == 1)
	{
		if (cut)
		{
			threshold(tmp, tmp, 255, 255, CV_THRESH_TRUNC);
			threshold(tmp, tmp, 0, 0, CV_THRESH_TOZERO);
		}
		else
			normalize(tmp, tmp, 0, 255, CV_MINMAX);

		tmp.convertTo(tmp, CV_8UC1);
	}
	else
	{
		tmp.convertTo(tmp, CV_8UC3);
	}
	imshow(win, tmp);
}

// function calls basic testing routines to test individual functions for correctness
void Dip4::test(void)
{

	test_circShift();
	cout << "Press enter to continue" << endl;
	cin.get();
}

void Dip4::test_circShift(void)
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
		cout << "ERROR: Dip4::circShift(): Result of circshift seems to be wrong!" << endl;
		return;
	}
	cout << "Message: Dip4::circShift() seems to be correct" << endl;
}
