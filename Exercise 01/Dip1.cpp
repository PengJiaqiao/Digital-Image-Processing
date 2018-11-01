//============================================================================
// Name        : Dip1.cpp
// Author      : Ronny Haensch
// Version     : 2.0
// Copyright   : -
// Description :
//============================================================================

#include "Dip1.h"

// function that performs some kind of (simple) image processing
/*
img	input image
return	output image
*/
Mat Dip1::doSomethingThatMyTutorIsGonnaLike(const Mat &img)
{
	//Implement defogging based on dark channel
	//Reference: He, Kaiming, Jian Sun, and Xiaoou Tang. "Single image haze removal using dark channel prior." IEEE transactions on pattern analysis and machine intelligence 33.12 (2011): 2341-2353.
	//Reference: http://coderskychen.cn/2015/12/11/%E6%9A%97%E9%80%9A%E9%81%93%E5%8E%BB%E9%9B%BE%E7%AE%97%E6%B3%95%E7%9A%84C-%E5%AE%9E%E7%8E%B0%E4%B8%8E%E4%BC%98%E5%8C%96%EF%BC%88%E4%B8%80%EF%BC%89/
	//Step 1: compute the dark channel of the original image
	int min = 255;
	Mat dark_channel_img(img.rows, img.cols, CV_8UC1);
	int rows = img.rows;
	int cols = img.cols;
	int b, g, r;

	for (int i = 0; i < rows; i++)
	{
		const uchar *inData = img.ptr<uchar>(i);
		uchar *outData = dark_channel_img.ptr<uchar>(i);
		for (int j = 0; j < cols; j++)
		{
			b = *inData++;
			g = *inData++;
			r = *inData++;
			min = min > b ? b : min;
			min = min > g ? g : min;
			min = min > r ? r : min;
			*outData++ = min;
			min = 255;
		}
	}

	//Step 1-1: minimum filter
	//Reference: https://blog.csdn.net/cgqzu/article/details/79888115?utm_source=blogxgwz1
	int windowsize = 15;
	r = (windowsize - 1) / 2; //radius
	Mat dst_ex;
	copyMakeBorder(dark_channel_img, dst_ex, r, r, r, r, BORDER_CONSTANT, Scalar(255));

	for (int i = r; i < dst_ex.rows - r; i++)
	{
		for (int j = r; j < dst_ex.cols - r; j++)
		{
			int minVal = dst_ex.at<uchar>(i, j);
			for (int s = -r; s < r + 1; s++)
			{
				for (int t = -r; t < r + 1; t++)
				{
					if (dst_ex.at<uchar>(i + s, j + t) < minVal)
					{
						minVal = dst_ex.at<uchar>(i + s, j + t);
					}
				}
			}
			dark_channel_img.at<uchar>(i - r, j - r) = minVal;
		}
	}

	//imwrite("dark_channel.jpg", dark_channel_img);

	//Step 2: computer the atmospheric light A
	int darksize = rows * cols;
	int topsize = darksize / 1000;
	int *A = new int[3];
	int sum[3] = {0, 0, 0};
	typedef struct Pixel
	{
		int x;
		int y;
		int value;
	} Pixel;
	Pixel *toppixels, *allpixels;
	toppixels = new Pixel[topsize];
	allpixels = new Pixel[darksize];

	for (int i = 0; i < rows; i++)
	{
		const uchar *outData = dark_channel_img.ptr<uchar>(i);
		for (int j = 0; j < cols; j++)
		{
			allpixels[i * cols + j].value = *outData++;
			allpixels[i * cols + j].x = i;
			allpixels[i * cols + j].y = j;
		}
	}
	std::sort(allpixels, allpixels + darksize, [](const Pixel &a, const Pixel &b) { return a.value > b.value; });

	memcpy(toppixels, allpixels, (topsize) * sizeof(Pixel)); //Finding out the lightest 1000 pixels
	int val0, val1, val2, avg, max = 0, maxi, maxj, x, y;
	for (int i = 0; i < topsize; i++)
	{
		x = allpixels[i].x;
		y = allpixels[i].y;
		const uchar *outData = img.ptr<uchar>(x);
		outData += 3 * y;
		val0 = *outData++;
		val1 = *outData++;
		val2 = *outData++;
		avg = (val0 + val1 + val2) / 3;
		if (max < avg)
		{
			max = avg;
			maxi = x;
			maxj = y;
		}
	}
	for (int i = 0; i < 3; i++)
	{
		A[i] = img.at<Vec3b>(maxi, maxj)[i];
		//A[i]=img.at<Vec4b>(maxi,maxj)[i];
		//A[i]=A[i]>220?220:A[i];
	}

	//Step 3: computer transmission t(X)
	float avg_A = (A[0] + A[1] + A[2]) / 3.0;
	float w = 0.95;
	Mat transmission(rows, cols, CV_32FC1);

	for (int k = 0; k < rows; k++)
	{
		const uchar *inData = dark_channel_img.ptr<uchar>(k);
		for (int l = 0; l < cols; l++)
		{
			transmission.at<float>(k, l) = 1 - w * (*inData++ / avg_A);
		}
	}

	Mat trans(rows, cols, CV_32FC1);
	Mat graymat(rows, cols, CV_8UC1);
	Mat graymat_normalization(rows, cols, CV_32FC1);
	cvtColor(img, graymat, CV_BGR2GRAY);
	for (int i = 0; i < rows; i++)
	{
		const uchar *inData = graymat.ptr<uchar>(i);
		for (int j = 0; j < cols; j++)
			graymat_normalization.at<float>(i, j) = *inData++ / 255.0;
	}
	//Step 3-1: guided image filtering (better than softmatting)
	//Reference: He, Kaiming, Jian Sun, and Xiaoou Tang. "Guided image filtering." European conference on computer vision. Springer, Berlin, Heidelberg, 2010.
	//Reference: https://blog.csdn.net/pi9nc/article/details/26592377
	Mat mat_Ip, mat_I2;
	multiply(graymat_normalization, transmission, mat_Ip);
	multiply(graymat_normalization, graymat_normalization, mat_I2);

	Mat mean_p, mean_I, mean_Ip, mean_I2;
	Size win_size(2 * r + 1, 2 * r + 1);
	boxFilter(transmission, mean_p, CV_32F, win_size);
	boxFilter(graymat_normalization, mean_I, CV_32F, win_size);
	boxFilter(mat_Ip, mean_Ip, CV_32F, win_size);
	boxFilter(mat_I2, mean_I2, CV_32F, win_size);

	Mat cov_Ip = mean_Ip - mean_I.mul(mean_p);
	Mat var_I = mean_I2 - mean_I.mul(mean_I);
	var_I += 0.001;

	Mat a_i, b_i;
	divide(cov_Ip, var_I, a_i);
	b_i = mean_p - a_i.mul(mean_I);

	Mat mean_a, mean_b;
	boxFilter(a_i, mean_a, CV_32F, win_size);
	boxFilter(b_i, mean_b, CV_32F, win_size);

	trans = mean_a.mul(graymat_normalization) + mean_b;

	//Step 4: compute the scene radiance J(X)
	float t = trans.at<float>(r, r);
	float t0 = 0.1;
	Mat final_img = Mat::zeros(rows, cols, CV_8UC3);
	int val = 0;
	copyMakeBorder(img, dst_ex, r, r, r, r, BORDER_CONSTANT, Scalar(255));
	for (int i = 0; i < 3; i++)
	{
		for (int k = 0; k < rows; k++)
		{
			const float *inData = trans.ptr<float>(k);
			const uchar *srcData = dst_ex.ptr<uchar>(k + r);
			srcData += r * 3 + i;
			uchar *outData = final_img.ptr<uchar>(k);
			outData += i;
			for (int l = 0; l < cols; l++)
			{
				t = *inData++;
				t = t > t0 ? t : t0;
				val = (int)((*srcData - A[i]) / t + A[i]);
				srcData += 3;
				val = val < 0 ? 0 : val;
				*outData = val > 255 ? 255 : val;
				outData += 3;
			}
		}
	}
	return final_img;
}

/* *****************************
  GIVEN FUNCTIONS
***************************** */

// function loads input image, calls processing function, and saves result
/*
fname	path to input image
*/
void Dip1::run(string fname)
{

	// window names
	string win1 = string("Original image");
	string win2 = string("Result");

	// some images
	Mat inputImage, outputImage;

	// load image
	cout << "load image" << endl;
	inputImage = imread(fname);
	cout << "done" << endl;

	// check if image can be loaded
	if (!inputImage.data)
	{
		cout << "ERROR: Cannot read file " << fname << endl;
		cout << "Press enter to continue..." << endl;
		cin.get();
		exit(-1);
	}

	// show input image
	namedWindow(win1.c_str());
	imshow(win1.c_str(), inputImage);

	// do something (reasonable!)
	outputImage = doSomethingThatMyTutorIsGonnaLike(inputImage);

	// show result
	namedWindow(win2.c_str());
	imshow(win2.c_str(), outputImage);

	// save result
	imwrite("result.jpg", outputImage);

	// wait a bit
	waitKey(0);
}

// function loads input image and calls the processing functions
// output is tested on "correctness"
/*
fname	path to the input image
*/
void Dip1::test(string fname)
{

	// some image variables
	Mat inputImage, outputImage;

	// load image
	inputImage = imread(fname);

	// check if image can be loaded
	if (!inputImage.data)
	{
		cout << "ERROR: Cannot read file " << fname << endl;
		cout << "Continue with pressing enter..." << endl;
		cin.get();
		exit(-1);
	}

	// create output
	outputImage = doSomethingThatMyTutorIsGonnaLike(inputImage);
	// test output
	test_doSomethingThatMyTutorIsGonnaLike(inputImage, outputImage);
}

// function loads input image and calls processing function
// output is tested on "correctness"
/*
inputImage	input image as used by doSomethingThatMyTutorIsGonnaLike(..)
outputImage	output image as created by doSomethingThatMyTutorIsGonnaLike(..)
*/
void Dip1::test_doSomethingThatMyTutorIsGonnaLike(const Mat &inputImage, const Mat &outputImage)
{

	vector<Mat> inputPlanes, outputPlanes;

	// ensure that input and output have equal number of channels
	// split (multi-channel) image into planes
	if ((inputImage.channels() == 3) and (outputImage.channels() == 1))
	{
		Mat input_tmp;
		cvtColor(inputImage, input_tmp, CV_BGR2GRAY);
		split(input_tmp, inputPlanes);
	}
	else
	{
		split(inputImage, inputPlanes);
	}
	split(outputImage, outputPlanes);

	// number of planes (1=grayscale, 3=color)
	int numOfPlanes = inputPlanes.size();

	// calculate and compare image histograms for each plane
	Mat inputHist, outputHist;
	// number of bins
	int histSize = 100;
	float range[] = {0, 256};
	const float *histRange = {range};
	bool uniform = true;
	bool accumulate = false;
	double sim = 0;
	for (int p = 0; p < numOfPlanes; p++)
	{
		// calculate histogram
		calcHist(&inputPlanes[p], 1, 0, Mat(), inputHist, 1, &histSize, &histRange, uniform, accumulate);
		calcHist(&outputPlanes[p], 1, 0, Mat(), outputHist, 1, &histSize, &histRange, uniform, accumulate);
		// normalize
		inputHist = inputHist / sum(inputHist).val[0];
		outputHist = outputHist / sum(outputHist).val[0];
		// similarity as histogram intersection
		sim += compareHist(inputHist, outputHist, CV_COMP_INTERSECT);
	}
	sim /= numOfPlanes;

	// check whether images are to similar after transformation
	if (sim >= 0.8)
		cout << "Warning: The input and output image seem to be quite similar (similarity = " << sim << " ). Are you sure your tutor is gonna like your work?" << endl;
}
