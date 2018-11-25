# DIGITAL IMAGE PROCESSING
## Introduction

Participants learn basic concepts, their theoretical foundation, and the most common algorithms used in digital image processing. After completing the module, participants understand strengths and limitations of different methods, are able to correctly and successfully apply methods and algorithms to real world problems, and are aware of performance criteria.

The homework will be programming exercises (C++ & OpenCV) that have to be handled in groups of 3-4 students.
## Group Member
[Peng Jiaqiao.](https://github.com/PengJiaqiao)  
[Long Zhou.](https://github.com/Joevonlong)   
[Zhang Liting.](https://github.com/Tingcaicai)    
[Fayanjuola Ayotomiwa.](https://github.com/tomfiger9)    
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
<img src="https://user-images.githubusercontent.com/26578566/48979068-bfa7eb80-f0b5-11e8-9169-a81da6e4498c.png" width="345">  
**Figure 3-2:** Implementation of four different smoothing methods, Spatial Domain Gaussian Filter - Frequency Domain Gaussian Filter - Separable Gaussian Filter - Box Filter with Integral Image (RGB convert to HSV and only work on value-channel)    
<img src="https://user-images.githubusercontent.com/26578566/48979120-88860a00-f0b6-11e8-89ed-01d129e2a0f6.jpg" width="1380">    
**Figure 3-3:** Enhanced Image, Spatial Domain Gaussian Filter - Frequency Domain Gaussian Filter - Separable Gaussian Filter - Box Filter with Integral Image  
<img src="https://user-images.githubusercontent.com/26578566/48979164-fcc0ad80-f0b6-11e8-8830-c5abbe5878ef.jpg" width="1380">    
#### TO DO  

● In function frequencyConvolution() in Dip3.cpp: expand the image to an optimal size to to achieve maximal DFT performance. Please read opencv doc carefully  (https://docs.opencv.org/2.4/doc/tutorials/core/discrete_fourier_transform/discrete_fourier_transform.html)  
● It seems that the smoothed image using frequency domain gaussian filter is different than others. It moves to top-left for several pixels. Can someone fix this little bug?  
● Prepare two graphs that describe time behaviour of convolution by usage of spatial and frequency domain  

ATTENTION: main.cpp owns some stupid mistakes. If you don't comment "produce difference image" like what I do, after a loop the source image Dip3.run() received is full of 0. So you will see only the first Spation Domain works and later you only get dark image. And even so every loop makes the source image gets lighter and the final result is the same. What I mean is the value of every pixels of input gets larger and larger. It's strange because in Dip3.cpp every input "Mat &src" comes with "const". I did some independent test for Dip3.cpp without main.cpp and it works perfectly. I will send an e-mail to TA.
