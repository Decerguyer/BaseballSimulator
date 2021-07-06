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
#include "Camera.h"
#include "D400.h"
#include "DataStructure.h"
#include "LocPredictor.h"
#include "Tracker.h"
#include "ThresholdFilter.h"
#include "Visualizer.h"
#include "DataStructure.h"
#include "sendPost.hpp"


int main(){

    //****************************Camera Calibration***************************//
    Camera calibrationParameters;
    cv::Mat rotationMatrix,translationMatrix;
    rotationMatrix = calibrationParameters.getRotationMatrix();
    translationMatrix = calibrationParameters.getTranslationMatrix();
    std::cout << "Rotation Matrix: " << rotationMatrix << std::endl;
    std::cout << "Translation Matrix: " << translationMatrix << std::endl;
    
    while(true){
        char yesNo;
        std::cout << "Restart Process? (Y/N)" << std::endl;
        std::cin >> yesNo;
        if (yesNo == 'N')
            break;

        //****************************Camera Initialization***************************//
        D400 cam;
        cam.enableStreams(848, 480, 90);
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
        data.centerPositions(calibrationParameters);
        std::cout << "Center Positions good\n";
        //data.centerError(calibrationParameters);
        //std::cout << "Center error good\n";
        data.transformPositions(calibrationParameters);
        std::cout << "transform positions good\n";
        data.transformError(calibrationParameters);
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
}

