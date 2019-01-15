//============================================================================
// Name        : Dip5.cpp
// Author      : Ronny Haensch
// Version     : 2.0
// Copyright   : -
// Description : 
//============================================================================

#include "Dip5.h"

// uses structure tensor to define interest points (foerstner)
void Dip5::getInterestPoints(const Mat& img, double sigma, vector<KeyPoint>& points){
	// TO DO !!!
	Mat Gx, Gy, Gx_sq, Gy_sq, Gx_Gy ;
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
	GaussianBlur(Gx_sq, Gx_sq, Size(5,5),0,0);
	GaussianBlur(Gy_sq, Gy_sq, Size(5,5),0,0);
	GaussianBlur(Gx_Gy, Gx_Gy, Size(5,5),0,0);
	

	//trace & det of structure tensor
	Mat trace = Gx_sq + Gy_sq;
	Mat det = Gx_sq .mul(Gy_sq) - Gx_Gy.mul(Gx_Gy); 
	
	//weight 
	Mat weight;
	divide(det, trace, weight);

	//weight non-max supression
	weight = nonMaxSuppression(weight);
	Scalar w_mean = mean(weight); 
	int w_min = w_mean.val[0] * 0.5; //changeable
	
	//weight threshold
	threshold(weight * -1, weight, w_min*-1, 255, THRESH_TRUNC);
	weight *= -1;

	//isotropy
	Mat q;
	divide(4*det, trace.mul(trace), q);
	
	//isotropy non-max supression
	q = nonMaxSuppression(q);
	float q_min = 0.5; //changeable
	
	//isotropy threshold
	threshold(q * -1, q, q_min*-1, 0, THRESH_TRUNC);
	q *= -1;
	
	//keypoints
	for (int i = 0; i< img.rows; i++ ){
		for(int j = 0; j< img.cols; j++){
			if(weight.at<float>(i, j) > w_min && q.at<float>(i, j) > q_min){
				KeyPoint point = KeyPoint(j,i,1);
				points.push_back(point);
			}
		

		}
	} 
	
}

// creates kernel representing fst derivative of a Gaussian kernel in x-direction
/*
sigma	standard deviation of the Gaussian kernel
return	the calculated kernel
*/
Mat Dip5::createFstDevKernel(double sigma){
	// TO DO !!!
	int window_diameter = (int)(2 * 3 * sigma + 0.5); // radius is 3*sigma empirical
	if (window_diameter%2==0)window_diameter += 1;
	Mat kernel = Mat::zeros(window_diameter,window_diameter,CV_32FC1);
	int center = (window_diameter-1) / 2; 
	float sigma_sq = (float)sigma * (float)sigma;
	
	for (int i = 0; i< window_diameter; i++ ){
		int x_dist = center - i;
		for(int j = 0; j< window_diameter; j++){
			int y_dist = center - j;
			kernel.at<float>(j,i) = -x_dist * exp(-(x_dist*x_dist + y_dist * y_dist)/2/sigma_sq) \
				/ 2 / CV_PI / sigma_sq / sigma_sq;
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
void Dip5::run(const Mat& in, vector<KeyPoint>& points){
   this->getInterestPoints(in, this->sigma, points);
}

// non-maxima suppression
// if any of the pixel at the 4-neighborhood is greater than current pixel, set it to zero
Mat Dip5::nonMaxSuppression(const Mat& img){

	Mat out = img.clone();
	
	for(int x=1; x<out.cols-1; x++){
		for(int y=1; y<out.rows-1; y++){
			if ( img.at<float>(y-1, x) >= img.at<float>(y, x) ){
				out.at<float>(y, x) = 0;
				continue;
			}
			if ( img.at<float>(y, x-1) >= img.at<float>(y, x) ){
				out.at<float>(y, x) = 0;
				continue;
			}
			if ( img.at<float>(y, x+1) >= img.at<float>(y, x) ){
				out.at<float>(y, x) = 0;
				continue;
			}
			if ( img.at<float>( y+1, x) >= img.at<float>(y, x) ){
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
void Dip5::showImage(const Mat& img, const char* win, int wait, bool show, bool save){
  
    Mat aux = img.clone();

    // scale and convert
    if (img.channels() == 1)
		normalize(aux, aux, 0, 255, CV_MINMAX);
		aux.convertTo(aux, CV_8UC1);
    // show
    if (show){
      imshow( win, aux);
      waitKey(wait);
    }
    // save
    if (save)
      imwrite( (string(win)+string(".png")).c_str(), aux);
}
