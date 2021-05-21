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

//Custom JEY Solutions headers
#include "BaseballSimulator.h"
#include "Camera.h"
#include "ImageData.h"
#include "CameraCalibration.h"
#include "DataStructure.h"
#include "Post.h"
#include "LocPredictor.h"
#include "Tracker.h"
#include "ThresholdFilter.h"
#include "Visualize.h"
#include "DataStructure.h"
#include "Post.hpp"


int main(){

    //****************************Camera Calibration***************************//
    CameraCalibration calibrationParameters;
    cv::Mat rotationMatrix,translationMatrix;
    rotationMatrix = calibrationParameters.getRotationMatrix();
    translationMatrix = calibrationParameters.getTranslationMatrix();
    std::cout << "Rotation Matrix: " << rotationMatrix << std::endl;
    std::cout << "Translation Matrix: " << translationMatrix << std::endl;
    
    //************************Data Structure Initialization***********************//
    data DataStructure;
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
    
    //*********************Position&Error Transformation Block********************//
    
    //****************************Spin Block***************************//
    
    //Deploy user input to input spin
    float[3] spin;
    std::cout<< "Enter Wb, Ws, Wg in order:" << std::endl;
    std::cin >> spin[0] >> spin[1] >> spin[2];
    std::cout << std::endl << "Spin values set: " << spin[0] << std::endl << spin[1] << std::endl << spin[2] << std::endl;
    data.data_struct.spin = spin;
    
    //************************Populate Data Structure Block***********************//
    
    //We have already filled in position, error, and spin at this point
    //Add Camera serial number and a temoprary pitcher name to the data structure
    
    
    //****************************JSONify data block***************************//
    //Jsonify the data using the jsonify method in DataStructure
    data.jsonObj = data.jsonify();
    
    //****************************HTTP Post block***************************//
    //Pass the JSON object to a POST object that will send the pitch to be recorded in the backend API
    Post httpPost;
    httpPost.Post(data.jsonObj);
    httpPost.sendRequest();
    
    //****************************Completion Message***************************//
    std::cout<<"The Routine has completed" << std::endl;
}

