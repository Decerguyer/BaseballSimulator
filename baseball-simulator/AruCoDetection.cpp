//Stationary Ball Measuring Tool below


//
//  stagnantMeasurement3D.cpp
//  baseball-test
//
//  Created by Yoni Arieh on 1/12/21.
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

int main(){
    
    //Camera Initialization Block
    Camera cam;    //Create Camera Object
    cam.enableStreams(848, 480, 90);    //Specify stream parameters
    cam.setDefaultSettings();
    //cam.disableEmitter();
    cam.setExposure(1000);
     
    //Recording Block
    std::cout << "Enter the integer number of frames to measure: ";    //Give user time to set up the ball
    int num;
    std::cin >> num;
    
    std::deque<ImageData> frames;
    frames = cam.recordImageData(num,30);    //(n,m) Record and save 'n' frames for measurements throw out first 'm' frames
    
    //Intrinsics Block
    auto intrin = cam.getIntrinsics(); //Retrieving camera intrinsics
    std::cout<<intrin.fx<<std::endl;
    std::cout<<intrin.fy<<std::endl;
    std::cout<<intrin.ppx<<std::endl;
    std::cout<<intrin.ppy<<std::endl;
    std::cout<<intrin.coeffs[0]<<intrin.coeffs[1]<<intrin.coeffs[2]<<intrin.coeffs[3]<<intrin.coeffs[4]<<std::endl;
    cv::Mat cameraMatrix = (cv::Mat_<float>(3,3) << 422.353,0,419.783,0,422.353,239.109,0,0,1); //Camera intrinsics combine with RS2_Intrin to make automatic script instead of manual
    cv::Mat distCoeffs = (cv::Mat_<float>(1,5) << 0,0,0,0,0); //Distortion coeff, should be 0 for all usable realsense cameras
    
    for(int i =0; i<(int)frames.size();(i >= 0 ? i++ : i = 0)){    //Iterates through the vector of framesets
    std::cout<<i<<std::endl;
    //Frame management block
    cv::Mat irMAT = frames[i].getIRMat();
    // Finding checker board corners
    // If desired number of corners are found in the image then success = true
        
    //Brightness Adjustment
    /*float gamma_ = 0.55;
    cv::Mat lookUpTable(1, 256, CV_8U);
    uchar* p = lookUpTable.ptr();
    for( int i = 0; i < 256; ++i)
        p[i] = cv::saturate_cast<uchar>(pow(i / 255.0, gamma_) * 255.0);
    
    cv::Mat brightened = frame.clone();
    cv::LUT(frame, lookUpTable, brightened);*/
    
    cv::Mat brightened;
    cv::equalizeHist(irMAT, brightened);
    
    cv::Mat frame;
    //cv::Mat extraSharpnessKernel = (cv::Mat_<float>(3,3) << -1,-1,-1,-1,9,-1,-1,-1,-1); /*Delete*/
    //cv::filter2D(irMAT,frame,-1,extraSharpnessKernel); /*Delete*/
    //frame = irMAT;
    frame = brightened;
    
    cv::Mat detectionMat,axesMat;
    cv::cvtColor(frame,detectionMat,cv::COLOR_GRAY2BGR);
    cv::cvtColor(frame,axesMat,cv::COLOR_GRAY2BGR);
    
    //Detection Block
    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
    cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_100);
    cv::aruco::detectMarkers(frame, dictionary, markerCorners, markerIds, parameters, rejectedCandidates);
    cv::aruco::drawDetectedMarkers(detectionMat, markerCorners, markerIds);
    /*
    //Pose Estimation Block
    std::vector<cv::Vec3d> rvecs, tvecs;
    cv::aruco::estimatePoseSingleMarkers(markerCorners, 28.57, cameraMatrix, distCoeffs, rvecs, tvecs);
    for (int i = 0; i < rvecs.size(); ++i) {
    	auto rvec = rvecs[i];
    	auto tvec = tvecs[i];
    	cv::aruco::drawAxis(axesMat, cameraMatrix, distCoeffs, rvec, tvec, 14.285);
    }
    
    //Cropped Block
    cv::Mat croppedFrame;
    croppedFrame = cv::Mat(frame,cv::Rect(212,120,212,120));
    
    //Image Scaling Block
    cv::Mat scaledFrame;
    cv::resize(croppedFrame, scaledFrame, cv::Size(), 4, 4, cv::INTER_CUBIC);
    cv::Mat scaledDetectionMat,scaledAxesMat;
    cv::cvtColor(scaledFrame,scaledDetectionMat,cv::COLOR_GRAY2BGR);
    cv::cvtColor(scaledFrame,scaledAxesMat,cv::COLOR_GRAY2BGR);
    
    //Sharpness Block
    cv::Mat sharpenedFrame;
    //cv::Mat sharpnessKernel = (cv::Mat_<float>(3,3) << -1,-1,-1,-1,9,-1,-1,-1,-1); //Small kernel
    cv::Mat sharpnessKernel = (cv::Mat_<float>(5,5) << -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,25,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1); //Larger Kernel
    cv::filter2D(scaledFrame,sharpenedFrame,-1,sharpnessKernel);
    
    //Scaled Detection Block
    std::vector<int> scaledMarkerIds;
    std::vector<std::vector<cv::Point2f>> scaledMarkerCorners, scaledRejectedCandidates;
    cv::aruco::detectMarkers(scaledFrame, dictionary, scaledMarkerCorners, scaledMarkerIds, parameters, scaledRejectedCandidates);
    cv::aruco::drawDetectedMarkers(scaledDetectionMat, scaledMarkerCorners, scaledMarkerIds);
    
    //Scaled Pose Estimation Block
    std::vector<cv::Vec3d> scaledRvecs, scaledTvecs;
    cv::aruco::estimatePoseSingleMarkers(scaledMarkerCorners, 28.57, cameraMatrix, distCoeffs, scaledRvecs, scaledTvecs);
    for (int i = 0; i < scaledRvecs.size(); ++i) {
    	auto scaledRvec = scaledRvecs[i];
    	auto scaledTvec = scaledTvecs[i];
    	cv::aruco::drawAxis(scaledAxesMat, cameraMatrix, distCoeffs, scaledRvec, scaledTvec, 14.285);
    }
    
    //Sharpness Detection Block
    cv::Mat sharpenedDetectionMat,sharpenedAxesMat;
    cv::cvtColor(sharpenedFrame,sharpenedDetectionMat,cv::COLOR_GRAY2BGR);
    cv::cvtColor(sharpenedFrame,sharpenedAxesMat,cv::COLOR_GRAY2BGR);
    
    std::vector<int> sharpenedMarkerIds;
    std::vector<std::vector<cv::Point2f>> sharpenedMarkerCorners, sharpenedRejectedCandidates;
    cv::aruco::detectMarkers(sharpenedFrame, dictionary, sharpenedMarkerCorners, sharpenedMarkerIds, parameters, sharpenedRejectedCandidates);
    cv::aruco::drawDetectedMarkers(sharpenedDetectionMat, sharpenedMarkerCorners, sharpenedMarkerIds);
    
    //Rejected Marker Block
    /*(for(int i = 0; i < rejectedCandidates.size(); i++){
    	std::cout << rejectedCandidates[0] << std::endl;
    }
    for(int i = 0; i < scaledRejectedCandidates.size(); i++){
    	std::cout << scaledRejectedCandidates[0] << std::endl;
    }*/
    /*cv::aruco::drawDetectedMarkers(sharpenedDetectionMat, sharpenedRejectedCandidates, sharpenedMarkerIds);
    for(int i = 0; i < sharpenedRejectedCandidates.size(); i++){
    	std::cout << sharpenedRejectedCandidates[0] << std::endl;
    }*/  
    
    //Visualization Block
    cv::imshow("Image", irMAT);
    cv::imshow("Brightened", brightened);
    cv::imshow("Detection" , detectionMat);
    cv::imshow("Axes" , axesMat);
    //cv::imshow("Scaled", scaledFrame);
    //cv::imshow("Scaled Detection" , scaledDetectionMat);
    //cv::imshow("Scaled Axes" , scaledAxesMat);
    //cv::imshow("Sharpened", sharpenedFrame);
    //cv::imshow("Sharpened Detection", sharpenedDetectionMat);
    int key = cv::waitKey(0);
        if (key == 'n'){
            if (i == (int)frames.size() - 1)
                i--;
            else
                continue;
        }
        else if (key == 27)
            break;
        else
            i -= 2;
    }
    cv::destroyAllWindows();
    cv::waitKey(1);
    frames.clear();      
}
