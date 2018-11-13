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
Implementation of Spatial Convolution, Average Filter, Median Filter and Bilateral Filter. Besides, Non-local Means filter is optional.   
**Figure 2-1:** Original - Shot Noise - Average Filter - Median Filter - Bilateral Filter - Non-local Means Filter.  
<img src="https://user-images.githubusercontent.com/26578566/48384470-d419da80-e6ea-11e8-8d83-729eb953627e.jpg" width="1350">
**Figure 2-2:** Original - Gaussion Noise - Average Filter - Median Filter - Bilateral Filter - Non-local Means Filter.   
<img src="https://user-images.githubusercontent.com/26578566/48384518-15aa8580-e6eb-11e8-98da-9b791f12f34d.jpg" width="1350">   
