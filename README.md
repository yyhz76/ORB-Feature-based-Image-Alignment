# ORB-Feature-based-Image-Alignment
Image alignment based on ORB (Oriented FAST and rotated BRIEF) features to restore color image

## Introduction

In early 1900s, photos could only be taken in grayscale. To get a color image, people used blue/green/red filters to filter out other colors in the light and obtained 3 grayscale photos representing the results of blue/green/red channels (Top). These 3 images were then merged to get a color image. However, due to the mechanical nature of the camera, the 3 images were usually misaligned. Simply superimposing them onto each other couldn't get the desired color image (Bottom Left). This project uses ORB (Oriented FAST and rotated BRIEF) feature-based image alignment algorithms (Center) to align these 3 images and restore a desirable RGB color image (Bottom Right).



![alt text](https://github.com/yyhz76/ORB-Feature-based-Image-Alignment/blob/main/images/BGR_channels_in_grayscale.png)<br />  
![alt text](https://github.com/yyhz76/ORB-Feature-based-Image-Alignment/blob/main/images/ORB_feature_matching_between_blue_and_green.png)<br />  
![alt text](https://github.com/yyhz76/ORB-Feature-based-Image-Alignment/blob/main/images/result_comparison.png)<br />  


<br /><br />
Image and video copyright belongs to https://learnopencv.com/
