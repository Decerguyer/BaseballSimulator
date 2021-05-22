//Stationary Ball Measuring Tool below


//
//  testTracker.cpp
//  baseball-test
//
//

#include <stdio.h>
#include "BaseballSimulator.h"
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <iostream>             // for cout
#include <vector>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/aruco.hpp"
#include "Camera.h"
#include "ImageData.h"
#include "ThresholdFilter.h"
#include "LocPredictor.h"
#include "Tracker.h"
#include "Visualizer.h"

int main(){
  //****************************Camera Initialization***************************//
    Camera cam;
    cam.enableStreams(848, 480, 90);
    cam.setExposure(10000); //Add method to change this from hard coded value?
    
    //**************************Threshold Initialization*************************//
    ThresholdFilter threshFilter(cam);
    
    //**************************Tracker Initialization*************************//
    Tracker trk(848, 480, cam.getIntrinsics(), threshFilter);

    //****************************User Control Block***************************//
    int numFrames;
    std::cout << "Enter number of frames to record\n";
    std::cin >> numFrames;
    std::deque<ImageData> images = cam.recordImageData(numFrames);

    //****************************Image Processing***************************//
    
    //Perhaps Deploy another User Control Block
    //to find the first frame the ball is in FOV

    std::vector<coord2D> coord2DVec;
    std::vector<long long> timeStampsVec;
    coord2DVec.reserve(images.size());
    timeStampsVec.reserve(images.size());
    for (int i = 0; i < images.size(); i++){        
        coord2DVec.push_back(trk.track(images[i]));
        timeStampsVec.push_back(images[i].getTimeStamp());
        std::cout << i << "\n";
    }
    std::cout << std::endl;
    
    //**************************Visualization Block *************************//
    Visualizer vis;
    
    //Change images to VALID images so only the frames with the ball are shown?
    vis.visualize(images, true, true, true);

    //**************************User Control Crop Block *************************//
    int startFrame, endFrame;
    std::cout<< "Enter the starting frame and then the ending frame (inclusive)" << std::endl;
    std::cin >> startFrame >> endFrame;
    coord2DVec.erase(coord2DVec.begin()+endFrame+1, coord2DVec.end());
    timeStampsVec.erase(timeStampsVec.begin()+endFrame+1, timeStampsVec.end());
    coord2DVec.erase(coord2DVec.begin(), coord2DVec.begin()+startFrame);
    timeStampsVec.erase(timeStampsVec.begin(), timeStampsVec.begin()+startFrame);

    //**************************Pixel to Positions Block *************************//
    std::vector<std::vector<float>> positions3D = trk.convertTo3D(coord2DVec);

    //**************************Print Ball Positions *************************//
    std::cout << positions3D.size() << " Ball positions found; Ball Positions Are: \n";
    for (int i = 0; i < positions3D.size(); i++){
        for (int k = 0; k < 3; k++){
            std::cout << positions3D[i][k] << " ";
        }
        std::cout << "\n";
    }
    return 0;
}

