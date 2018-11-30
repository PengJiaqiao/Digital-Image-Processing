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
Mat Dip4::circShift(const Mat& in, int dx, int dy){
	Mat rst = in.clone();
	int new_row,new_col;

	for(int p=0;p<in.rows;p++){
		new_row = p + dy;
		if (new_row<0){new_row += in.rows;}
		else if(new_row >= in.rows){new_row -= in.rows;}
		for(int q=0;q<in.cols;q++){
			new_col = q + dy;
			if (new_col<0){new_col += in.cols;}
			else if(new_col >= in.cols){new_col -= in.cols;}
			rst.at<float>(new_row,new_col) = in.at<float>(p,q);
		}
	}
	
	return rst.clone();
   
}

// Function applies the inverse filter to restorate a degraded image
/*
degraded :  degraded input image
filter   :  filter which caused degradation
return   :  restorated output image
*/
Mat Dip4::inverseFilter(const Mat& degraded, const Mat& filter){
	// TO DO !!!
   return degraded;
}

// Function applies the wiener filter to restorate a degraded image
/*
degraded :  degraded input image
filter   :  filter which caused degradation
snr      :  signal to noise ratio of the input image
return   :   restorated output image
*/
Mat Dip4::wienerFilter(const Mat& degraded, const Mat& filter, double snr){
	// TO DO !!!

	//copy filter to larger template
	Mat big_filter = 0.*degraded.clone();
	Mat mask = big_filter(Rect(0,0,filter.rows,filter.cols));
	filter.copyTo(mask);
	big_filter = circShift(big_filter.clone(),-1*int(filter.cols/2),-1*int(filter.rows/2));

	//pad zero to the second channel
	Mat big_filter_pad,img_pad;
	Mat pad = 0.*degraded.clone();
	vector<Mat>pad_(2);
	pad_[0] = big_filter.clone();
	pad_[1] = pad.clone();
	merge(pad_, big_filter_pad);

	vector<Mat>pad_img_(2);
	pad_img_[0] = degraded.clone();
	pad_img_[1] = pad.clone();
	merge(pad_img_, img_pad);

	//dft
	Mat dft_filter,dft_img;
	dft(big_filter_pad,dft_filter);
	dft(img_pad,dft_img);
	
	//conjugate Pk_star
	Mat filt_conj = 0.*dft_filter.clone();

	vector<Mat>split_(2);
	split(dft_filter, split_);
	Mat real = split_[0].clone();
	Mat imag = split_[1].clone();
	imag *= -1.;
	
	//denominator

	Mat denom;
	denom = real.mul(real) + imag.mul(imag);
	denom += 1/snr/snr;
	
	//division
	Mat realq, imagq;
	divide(real, denom, realq);
	divide(imag, denom, imagq);
	
	//apply filter
	Mat dft_rst, rst;

	vector<Mat>img_split_(2);
	split(dft_img, img_split_);

	vector<Mat>merge_rst_(2);
	merge_rst_[0] = img_split_[0].mul(realq) - img_split_[1].mul(imagq);
	merge_rst_[1] = img_split_[1].mul(realq) + img_split_[0].mul(imagq);
	merge(merge_rst_, dft_rst);
	dft(dft_rst,rst,DFT_INVERSE + DFT_SCALE);
	
	//de-padding
	vector<Mat>img_depad_(2);
	split(rst,img_depad_);
	Mat rst0 = img_depad_[0].clone();

	//threshold image value to 0-255
	threshold(rst0,rst0,255,255,THRESH_TRUNC);
	
   return rst0.clone();
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
Mat Dip4::run(const Mat& in, string restorationType, const Mat& kernel, double snr){

   if (restorationType.compare("wiener")==0){
      return wienerFilter(in, kernel, snr);
   }else{
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
Mat Dip4::degradeImage(const Mat& img, Mat& degradedImg, double filterDev, double snr){

    int kSize = round(filterDev*3)*2 - 1;
   
    Mat gaussKernel = getGaussianKernel(kSize, filterDev, CV_32FC1);
    gaussKernel = gaussKernel * gaussKernel.t();

    Mat imgs = img.clone();
    dft( imgs, imgs, CV_DXT_FORWARD, img.rows);
    Mat kernels = Mat::zeros( img.rows, img.cols, CV_32FC1);
    int dx, dy; dx = dy = (kSize-1)/2.;
    for(int i=0; i<kSize; i++) for(int j=0; j<kSize; j++) kernels.at<float>((i - dy + img.rows) % img.rows,(j - dx + img.cols) % img.cols) = gaussKernel.at<float>(i,j);
	dft( kernels, kernels, CV_DXT_FORWARD );
	mulSpectrums( imgs, kernels, imgs, 0 );
	dft( imgs, degradedImg, CV_DXT_INV_SCALE, img.rows );
	
    Mat mean, stddev;
    meanStdDev(img, mean, stddev);

    Mat noise = Mat::zeros(img.rows, img.cols, CV_32FC1);
    randn(noise, 0, stddev.at<double>(0)/snr);
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
void Dip4::showImage(const char* win, const Mat& img, bool cut){

   Mat tmp = img.clone();

   if (tmp.channels() == 1){
      if (cut){
         threshold(tmp, tmp, 255, 255, CV_THRESH_TRUNC);
         threshold(tmp, tmp, 0, 0, CV_THRESH_TOZERO);
      }else
         normalize(tmp, tmp, 0, 255, CV_MINMAX);
         
      tmp.convertTo(tmp, CV_8UC1);
   }else{
      tmp.convertTo(tmp, CV_8UC3);
   }
   imshow(win, tmp);
}

// function calls basic testing routines to test individual functions for correctness
void Dip4::test(void){

   test_circShift();
   cout << "Press enter to continue"  << endl;
   cin.get();

}

void Dip4::test_circShift(void){
   
   Mat in = Mat::zeros(3,3,CV_32FC1);
   in.at<float>(0,0) = 1;
   in.at<float>(0,1) = 2;
   in.at<float>(1,0) = 3;
   in.at<float>(1,1) = 4;
   Mat ref = Mat::zeros(3,3,CV_32FC1);
   ref.at<float>(0,0) = 4;
   ref.at<float>(0,2) = 3;
   ref.at<float>(2,0) = 2;
   ref.at<float>(2,2) = 1;
   
   if (sum((circShift(in, -1, -1) == ref)).val[0]/255 != 9){
      cout << "ERROR: Dip4::circShift(): Result of circshift seems to be wrong!" << endl;
      return;
   }
   cout << "Message: Dip4::circShift() seems to be correct" << endl;
}
