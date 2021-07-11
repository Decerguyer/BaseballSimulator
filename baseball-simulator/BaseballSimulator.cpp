//
//  BaseballSimulator.h
//
//  Created by Jordan Mayor on 1/8/21.
//  Revamped in V2 by Yoni Arieh on 7/3/21.

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include "Camera.h"
#include "D400.h"
#include "DataStructure.h"
//#include "FeatureAnalysis.h"
#include "ImageData.h"
#include "ImageReconditioning.h"
#include "json.hpp"
#include "LocPredictor.h"
#include "sendPost.h"
#include "ThresholdFilter.h"
#include "Tracker.h"
#include "Visualizer.h"
#include "BaseballSimulator.h"
#include "V2.h"

BaseballSimulator::BaseballSimulator(){
}

void BaseballSimulator::MainTestingRoutine(){
    
    //****************************Camera Initialization & Calibration***************************//
    D400 cam(ctx);
    std::cout << "Attached Camera" << endl;
    cam.calibrate();
    cam.setDefaultSettings();
    int exposure;
    std::cout << "Enter Exposure Value: ";
    std::cin >> exposure;
    cam.setExposure(exposure); //Add method to change this from hard coded value?
    
    //**************************Threshold Initialization*************************//
    ThresholdFilter threshFilter(cam);
    
    //**************************Tracker Initialization*************************//
    Tracker trk(848, 480, cam.getIntrinsics(), threshFilter);

    //****************************User Control Block***************************//
    int numFrames;
    std::cout << "Enter number of frames to record\n";
    std::cin >> numFrames;
    std::deque<ImageData> images = cam.recordImageData(numFrames);
    
    //************************Data Structure Initialization***********************//
    DataStructure data;

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
    }
    
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
    for (int i = 0; i < positions3D.size(); i++){
        if(positions3D[i][2]){ //If the position wasn't an accidental 0,0,0
            data.uncenteredPositions.push_back(positions3D[i]);
            data.timestamps.push_back(timeStampsVec[i]);
        }
    }

    std::cout << "Pixel to Positions Block Good\n";
    
    //***********************Position Error Processing Block**********************//
    data.setError();
    std::cout << "Position Error Processing Block Good\n";

    
    //*********************Position&Error Transformation Block********************//
    data.centerPositions();
    std::cout << "Center Positions good\n";
    data.transformPositions(cam);
    std::cout << "transform positions good\n";
    data.transformError(cam);
    std::cout << "transform error good\n";

    std::cout << "Position&Error Transformation Block Good\n";

    //****************************Spin Block***************************//
    
    //Deploy user input to input spin
    float spin[3];
    std::cout<< "Enter Wb, Ws, Wg in order:" << std::endl;
    std::cin >> spin[0] >> spin[1] >> spin[2];
    std::cout << std::endl << "Spin values set: \n" << spin[0] << std::endl << spin[1] << std::endl << spin[2] << std::endl;
    data.spin.push_back(spin[0]);
    data.spin.push_back(spin[1]);
    data.spin.push_back(spin[2]);
    
    //************************Populate Data Structure Block***********************//
    
    //We have already filled in position, error, and spin at this point
    //Add Camera serial number and a temoprary pitcher name to the data structure
    
    data.username = "JEYsolutions";
    data.serialNumber = 99999999;
    //****************************HTTP Post block***************************//
    //Pass the JSON object to a POST object that will send the pitch to be recorded in the backend API
    sendPost httpPost;
    httpPost.sendRequest(data.jsonify());
    
    //****************************Completion Message***************************//
    std::cout<<"The Routine has completed" << std::endl;
}

void BaseballSimulator::Save(){
    // D400 cam(ctx);
    // std::cout << "Attached Camera" << endl;
    // cam.calibrate();
    // cam.setDefaultSettings();
    // cam.writeMat("RotationMatrix");
}
void BaseballSimulator::Load(){
    // D400 cam(ctx);
    // std::cout << "Attached Camera" << endl;
    // cam.readMat("RotationMatrix");
}
