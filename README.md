# DIGITAL IMAGE PROCESSING
## Introduction

Participants learn basic concepts, their theoretical foundation, and the most common algorithms used in digital image processing. After completing the module, participants understand strengths and limitations of different methods, are able to correctly and successfully apply methods and algorithms to real world problems, and are aware of performance criteria.

The homework will be programming exercises (C++ & OpenCV) that have to be handled in groups of 3-4 students.
## Group Member
[Peng Jiaqiao.](https://github.com/PengJiaqiao)   
[Li Huiyuan.](https://github.com/robinmiali)
## Environment
Ubuntu 16.04, OpenCV 3.4.3
## Exercise
### Exercise 01
● [Install C++-compiler]  
● [Install OpenCV]  
● Dip1.cpp     
　　● Mat Dip1::doSomethingThatMyTutorIsGonnaLike(Mat&)  
　　　→ Implement defogging based on dark channel
### Demo
```
cd Exercise\ 01/
g++ -std=c++11 main.cpp Dip1.cpp `pkg-config --cflags --libs /usr/local/OpenCV-3.4.3/lib/pkgconfig/opencv.pc` -o main
./main input\ 2.png
```
**Figure 1-1:** Original    
<img src="https://user-images.githubusercontent.com/26578566/47862771-3115b680-ddf6-11e8-99d3-f37bec7b03e8.png" width="450">  
**Figure 1-2:** Dark Channel    
<img src="https://user-images.githubusercontent.com/26578566/47862781-3410a700-ddf6-11e8-8c38-178892b122e0.jpg" width="450">    
**Figure 1-3:** Output    
<img src="https://user-images.githubusercontent.com/26578566/47862946-85b93180-ddf6-11e8-9d31-8116507f17e9.jpg" width="450">

### Exercise 02
#### Noise Reduction  
Implementation of Spatial Convolution, Average Filter, Median Filter and Bilateral Filter. Besides, Non-local Means filter is optional.   
**Figure 2-1:** Original - Shot Noise - Average Filter - Median Filter - Bilateral Filter - Non-local Means Filter.  
<img src="https://user-images.githubusercontent.com/26578566/48384470-d419da80-e6ea-11e8-8d83-729eb953627e.jpg" width="1350">
**Figure 2-2:** Original - Gaussion Noise - Average Filter - Median Filter - Bilateral Filter - Non-local Means Filter.   
<img src="https://user-images.githubusercontent.com/26578566/48384518-15aa8580-e6eb-11e8-98da-9b791f12f34d.jpg" width="1350">   

### Exercise 03
#### Unsharp Masking
Subtract smoothed image from original, and add difference to original image.  
**Figure 3-1:** Original  
<img src="https://user-images.githubusercontent.com/26578566/48979068-bfa7eb80-f0b5-11e8-9169-a81da6e4498c.png" width="225">  
**Figure 3-2:** Implementation of four different smoothing methods, Spatial Domain Gaussian Filter - Frequency Domain Gaussian Filter - Separable Gaussian Filter - Box Filter with Integral Image (RGB convert to HSV and only work on value-channel)    
<img src="https://user-images.githubusercontent.com/26578566/48979120-88860a00-f0b6-11e8-89ed-01d129e2a0f6.jpg" width="1380">    
**Figure 3-3:** Enhanced Image, Spatial Domain Gaussian Filter - Frequency Domain Gaussian Filter - Separable Gaussian Filter - Box Filter with Integral Image  
<img src="https://user-images.githubusercontent.com/26578566/48979164-fcc0ad80-f0b6-11e8-8830-c5abbe5878ef.jpg" width="1380">    
#### Time behaviour
**Figure 3-4:** Time behaviour of convolution by usage of spatial domain (plot by python)  
● x-direction: filter size  
● y-direction: number of pixels in input image  
● z-direction: time  
<img src="https://user-images.githubusercontent.com/26578566/49160272-b1143b00-f326-11e8-8314-7d626d8c99af.png" width="640">   
**Figure 3-5:** Time behaviour of convolution by usage of frequency domain (plot by python)  
● x-direction: filter size  
● y-direction: number of pixels in input image  
● z-direction: time  
<img src="https://user-images.githubusercontent.com/26578566/49160444-0f411e00-f327-11e8-8431-9e4f8c582f20.png" width="640">
### Exercise 04
#### Image Restoration  
● Applies (modified) inverse filter to restore image (e.g. ε=0.05 )  
● Applies Wiener filter to restore image  
**Figure 4-1:** Original Image - Degraded Image - Inverse Filter - Wiener Filter  
<img src="https://user-images.githubusercontent.com/26578566/50699769-19062180-1049-11e9-9b93-b66ec741597f.jpg" width="1496">
### Exercise 05
#### Interest Points  
● Generates kernel that corresponds to the first derivative of a Gaussian  
● Computes keypoints using structure tensor (Förstner Operator)  
**Figure 5-1:** Original Image - Image with Keypoints  
<img src="https://user-images.githubusercontent.com/26578566/51217893-4ea0e980-192a-11e9-8335-4040f7940ed0.jpg" width="1004">
