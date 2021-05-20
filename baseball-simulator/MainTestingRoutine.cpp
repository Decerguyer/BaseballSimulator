//Test Program to fully utilize the hardware (without spin estimation)
//
//MainTestingRoutine.cpp
//
//JEY Solutions
//
//Created By: Yoni Arieh
//5/20/21
//

//Standard Includes
#include <stdio.h>
#include <iostream>

#include <vector> //I don't think we even use this in the main. Remove?

//OpenCV libraries. Remove?
#include <opencv2/calib3d/calib3d.hpp> //These should exist only in the headers of
#include <opencv2/highgui/highgui.hpp> //the classes where they are used. Remove?
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/aruco.hpp"

//Custom Intel Realsense Header
//This too can likely be removed and only exist in the Camera class that utilizes it?
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API

//Custom JEY Solutions headers
#include "BaseballSimulator.h"
#include "CameraCalibration.h"
#include "DataStructure.h"
#include "JsonifyData.h"
#include "Post.h"


int main(){

    //****************************Camera Calibration***************************//
    CameraCalibration calibrationParameters;
    cv::Mat rotationMatrix,translationMatrix;
    rotationMatrix = calibrationParameters.getRotationMatrix();
    translationMatrix = calibrationParameters.getTranslationMatrix();
    std::cout << "Rotation Matrix: " << rotationMatrix << std::endl;
    std::cout << "Translation Matrix: " << translationMatrix << std::endl;
    
    //****************************Camera Initialization***************************//
    Camera cam;
    cam.enableStreams(848, 480, 90);
    cam.setExposure(1500); //Add method to change this from hard coded value?
    
    //**************************Threshold Initialization*************************//
    ThresholdFilter threshFilter(cam);
    
    //**************************Tracker Initialization*************************//
    Tracker trk(848, 480, cam.getIntrinsics());

    //****************************User Control Block***************************//
    int numFrames;
    std::cout << "Enter number of frames to record\n";
    std::cin >> a;
    std::deque<ImageData> images = cam.recordImageData(numFrames);

    //****************************Image Processing***************************//
    
    //Perhaps Deploy another User Control Block
    //to find the first frame the ball is in FOV
    
    for (int i = 0; i < images.size(); i++){
        images[i].depthMat = threshFilter.filter(images[i].depthMat);
        images[i].depthVisMat = images[i].depthToVisual(images[i].depthMat);
        
        coord2D coordinate = trk.track(images[i]);

        std::cout << i << std::endl;
    }
    
    //**************************Visualization Block *************************//
    Visualizer vis;
    
    //Change images to VALID images so only the frames with the ball are shown?
    vis.visualize(images, true, true, true);
    
    //***********************Position Error Processing Block**********************//
    
    //****************************Spin Block***************************//
    
    //Deploy user input to input spin
    
    //************************Populate Data Structure Block***********************//
    
    //We may have already filled in position, error, and spin at this point?
    //Add Camera serial number and a temoprary pitcher name to the data structure
    
    //****************************JSONify data block***************************//
    //Pass data structure to a JSONify data object which will return a JSON object
    
    //****************************HTTP Post block***************************//
    //Pass the JSON object to a POST object that will send the pitch to be recorded in the backend API
}

