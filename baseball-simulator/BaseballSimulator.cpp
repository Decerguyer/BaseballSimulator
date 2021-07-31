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
    std::cout << "Square Size: " << SQUARE_SIZE << endl;
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

void BaseballSimulator::savingRoutine(std::string str){
    cv::FileStorage file(str, cv::FileStorage::WRITE);

    //****************************Camera Initialization & Calibration***************************//
    D400 cam(ctx);
    std::cout << "Attached Camera" << endl;
    std::cout << "Square Size: " << SQUARE_SIZE << endl;
    cam.calibrate();

    std::cout << "Saving Calibration\n";
    file << "CameraCalibration" << cam; //Saves Camera/D400 Object with Node Name "CameraCalibration"
    
    cam.setDefaultSettings();
    
    //**************************Threshold Initialization*************************//
    ThresholdFilter threshFilter(cam);
    std::cout << "Saving ThresholdFilter\n";
    file << "ThresholdFilter" << threshFilter;

    //****************************User Control Block***************************//
    int exposure;
    std::cout << "Enter Exposure Value: ";
    std::cin >> exposure;
    cam.setExposure(exposure); //Add method to change this from hard coded value?

    std::cout << "Saving Exposure\n";
    file << "Exposure" << exposure;


    int numFrames;
    std::cout << "Enter number of frames to record\n";
    std::cin >> numFrames;
    std::deque<ImageData> images = cam.recordImageData(numFrames);

    
    //**************************Visualization Block *************************//
    Visualizer vis;
    
    //Change images to VALID images so only the frames with the ball are shown?
    vis.visualize(images, true, true, true);

    //**************************User Control Crop Block *************************//
    int startFrame, endFrame;
    std::cout<< "Enter the starting frame and then the ending frame (inclusive)" << std::endl;
    std::cin >> startFrame >> endFrame;

    images.erase(images.begin()+endFrame+1, images.end());
    images.erase(images.begin(), images.begin()+startFrame);

    std::cout << "Saving Images\n";
    saveImageDataDeque(images, file);

    //****************************Spin Block***************************//
    
    //Deploy user input to input spin
    float spin[3];
    std::cout<< "Enter Wb, Ws, Wg in order:" << std::endl;
    std::cin >> spin[0] >> spin[1] >> spin[2];
    std::cout << std::endl << "Spin values set: \n" << spin[0] << std::endl << spin[1] << std::endl << spin[2] << std::endl;

    file << "Spin0" << spin[0]
    << "Spin1" << spin[1]
    << "Spin2" << spin[2];


    //****************************Ground Truth Block ***************************//
    std::cout << "Enter Ground Truth x, z\n";
    float x, z;
    cin >> x >> z;

    file << "GroundTruthX" << x
    << "GroundTruthZ" << z;

    std::cout<<"The Routine has completed\n" << std::endl;
}

void BaseballSimulator::loadingRoutine(std::string str){

    cv::FileStorage file(str, cv::FileStorage::READ);

    //****************************Camera Initialization & Calibration***************************//
    D400 cam(ctx);
    file["CameraCalibration"] >> cam;
    
    int exposure;
    file["Exposure"] >> exposure;
    
    //**************************Threshold Initialization*************************//
    ThresholdFilter threshFilter;
    file["ThresholdFilter"] >> threshFilter;

    //**************************Tracker Initialization*************************//
    Tracker trk(848, 480, cam.getIntrinsics(), threshFilter);
    //****************************User Control Block***************************//
    std::deque<ImageData> images;
    loadImageDataDeque(images, file);
    
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
    std::vector<coord2D> clicks;
    Visualizer vis(&clicks);
    
    //Change images to VALID images so only the frames with the ball are shown?
    vis.visualize(images, true, true, true);

    for(int l = 0; l < clicks.size(); l++){
        std::cout << "x: " << clicks[l].x << " y: " << clicks[l].y << " z: " << clicks[l].depth << std::endl;
    }

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
    file["Spin0"] >> spin[0];
    file["Spin1"] >> spin[1];
    file["Spin2"] >> spin[2];
    
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

void BaseballSimulator::photographyRoutine(){
    D400 cam(ctx);
    std::cout << "Attached Camera" << endl;
    cam.setDefaultSettings();
    
    //**************************Threshold Initialization*************************//
    ThresholdFilter threshFilter(cam);
    std::cout << "ThresholdFilter\n";

    //****************************User Control Block***************************//
    int exposure;
    std::cout << "Enter Exposure Value: ";
    std::cin >> exposure;
    cam.setExposure(exposure); //Add method to change this from hard coded value?

    int numFrames;
    std::cout << "Enter number of frames to record\n";
    std::cin >> numFrames;
    std::deque<ImageData> images = cam.recordImageData(numFrames);

    //**************************Visualization Block *************************//
    Visualizer vis;
    
    //Change images to VALID images so only the frames with the ball are shown?
    vis.visualize(images, true, true, true);

}

void BaseballSimulator::distFromWall(){
    D400 cam(ctx);
    std::cout << "Attached Camera" << endl;  
    std::cout << "Square Size: " << SQUARE_SIZE << endl;  
    cam.calibrate();

    cv::Mat R = cam.getRotationMatrix();
    cv::Mat T = cam.getTranslationMatrix();

    cv::Mat zeros = (cv::Mat_<double>(3,1) << 0,0,0);

    std::cout << "Rotation Inverse: " << R << std::endl;
    std::cout << "Translation: " << T << std::endl;
    std::cout << "(0-T)*(R^-1): " <<  R*(zeros-T) << std::endl;
}

void BaseballSimulator::singleMeasurement(){
    
    D400 cam(ctx);
    std::cout << "Attached Camera" << endl;
    std::cout << "Square Size: " << SQUARE_SIZE << endl;
    cam.calibrate();

    ThresholdFilter threshFilter(cam);

    cam.setDefaultSettings();

    int exposure;
    std::cout << "Enter Exposure Value: ";
    std::cin >> exposure;
    cam.setExposure(exposure); //Add method to change this from hard coded value?

    int numFrames;
    std::cout << "Enter number of frames to record\n";
    std::cin >> numFrames;
    std::deque<ImageData> images = cam.recordImageData(numFrames);


    std::vector<coord2D> clicks;
    Visualizer vis(&clicks);

    //Change images to VALID images so only the frames with the ball are shown?
    vis.visualize(images, true, true, true);

    for(int l = 0; l < clicks.size(); l++){
        std::cout << "Px: " << clicks[l].x << " Py: " << clicks[l].y << " z: " << clicks[l].depth << std::endl;
    }

    Tracker trk(848, 480, cam.getIntrinsics(), threshFilter);
    std::vector<std::vector<float>> positions3D = trk.convertTo3D(clicks);

    for(int l = 0; l < positions3D.size(); l++){
        std::cout << "x: " << positions3D[l][0] << " y: " << positions3D[l][1] << " z: " << positions3D[l][2] << std::endl;
    }

    DataStructure data;
    data.centerPositions();
    std::cout << "Center Positions good\n";
    data.transformPositions(cam);
    std::cout << "transform positions good\n";

    for(int l = 0; l < data.positions.size(); l++){
        std::cout << "x: " << data.positions[l][0] << " y: " << data.positions[l][1] << " z: " << data.positions[l][2] << std::endl;
    }

}