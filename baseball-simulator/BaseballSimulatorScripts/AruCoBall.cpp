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

std::vector <cv::Vec3f> ball_detector(cv::Mat image, double distance){
    std::vector<cv::Vec3f> circles;

    int radius = (24*0.646)/distance;
    
    if (radius > 25){
        radius = 25;
    }
    
    
    //cv::HoughCircles(image, circles, cv::HOUGH_GRADIENT, 1.6, 4000, 50, 5, (radius - radius*0.15), radius + radius*0.15);
    cv::HoughCircles(image, circles, cv::HOUGH_GRADIENT, 1.6, 4000, 50, 5, 5, 50);
    //std::cout<<circles.size()<<std::endl;
    return circles;
}

int main(){
    
    //Camera Initialization Block
    Camera cam;    //Create Camera Object
    cam.enableStreams(848, 480, 90);    //Specify stream parameters
    cam.setDefaultSettings();
    ThresholdFilter threshFilter;    //Create depth background for threshold/background-subraction
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
    
    //Frame management block
    cv::Mat irMAT = frames[i].getIRMat();
    cv::Mat depthMatOriginal = frames[i].getDepthMat(); //Pull the depth frame from the frameset and convert it to cv::Mat format
    depthMatOriginal = threshFilter.filter(depthMatOriginal,0.3048,3);    //Send the depth frame through the threshold/background-subtraction filter
    cv::Mat depthMatCopy;    //Create a copy of the filtered depth frames
    depthMatCopy = depthMatOriginal.clone();
    
    depthMatCopy.convertTo(depthMatCopy, CV_8UC1, 255,0); //Convert the copy to 8 bit grey-scale format
    
    cv::Mat gaussianBlur;
    cv::GaussianBlur(depthMatCopy,gaussianBlur,cv::Size(9,9),0);
    
    float minThreshDist = 0.3048f;
    
    
    //Ball Detection
    std::vector <cv::Vec3f> circles = ball_detector(/*depthMatCopy*/gaussianBlur,minThreshDist); //Find the ball in the image
    
    
    float pixel[2] = {90, 90}; //Temporary center of the ball
    if (circles.size() >= 1){ //If the vector circles has a value (ball is found)
          pixel[0] = (circles[0][0]) - (pixel[0] >= 90 ? 90 : pixel[0]) + pixel[0]; //Define the center of the ball column
          pixel[1] = (circles[0][1]) - (pixel[1] >= 90 ? 90 : pixel[1]) + pixel[1]; //Define the center of the ball row
          cv::Point pnt(pixel[0], pixel[1]); //Define the center point will be used to draw circle on top of the image
          std::cout<<"pixel column: " << pixel[0] << std::endl << "pixel row: " << pixel[1] << std::endl; //Output to user    
          float distance = depthMatOriginal.at<float>(pixel[1], pixel[0]); //Retrieve distance of center from original depth image in cv MAT
          cv::circle(depthMatOriginal, pnt, int(circles[0][2]), cv::Scalar(0,255,0), 2, 8, 0 ); //Draw a circle with the determined radius of the ball around ball's center point
          cv::circle(depthMatOriginal, pnt, 2 , cv::Scalar(0,0,255), 2, 8, 0 ); //Draw a point marking the center of the ball
    //}
    
    //Cropped Block
    cv::Mat croppedFrame;
    float wP = circles[0][2]*3;
    float pX1 = circles[0][0]-circles[0][2]*1.5;
    float pY1 = circles[0][1]-circles[0][2]*1.5;
    float pX2 = pX1 + wP;
    float pY2 = pY1 + wP;
    
    if(pX1 < 0){pX1=0; std::cout<<"Clamped Crop to Left Edge" << std::endl;}
    if(pX2 > 847){pX2=847;std::cout<<"Clamped Crop to Right Edge" << std::endl;}
    if(pY1 < 0){pY1=0;std::cout<<"Clamped Crop to Bottom Edge" << std::endl;}
    if(pY2 > 479){pY2=479;std::cout<<"Clamped Crop to Top Edge" << std::endl;}
    croppedFrame = cv::Mat(irMAT,cv::Rect(pX1,pY1,pX2-pX1,pY2-pY1));
    cv::imshow("Cropped", croppedFrame);
    
    cv::Mat croppedBrightened;
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(40);
    clahe->apply(croppedFrame,croppedBrightened);
    cv::imshow("Cropped & Brightened", croppedBrightened);
    
    //Detection Block
    cv::Mat detectionMat,axesMat;
    cv::cvtColor(croppedBrightened,detectionMat,cv::COLOR_GRAY2BGR);
    cv::cvtColor(croppedBrightened,axesMat,cv::COLOR_GRAY2BGR);
    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
    cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_100);
    cv::aruco::detectMarkers(croppedBrightened, dictionary, markerCorners, markerIds, parameters, rejectedCandidates);
    cv::aruco::drawDetectedMarkers(detectionMat, markerCorners, markerIds);
    
    
    //Pose Estimation Block
    std::vector<cv::Vec3d> rvecs, tvecs;
    cv::aruco::estimatePoseSingleMarkers(markerCorners, 28.57, cameraMatrix, distCoeffs, rvecs, tvecs);
    for (int i = 0; i < rvecs.size(); ++i) {
    	auto rvec = rvecs[i];
    	auto tvec = tvecs[i];
    	cv::aruco::drawAxis(axesMat, cameraMatrix, distCoeffs, rvec, tvec, 14.285);
    }
    
    //Uncropped Image user feedback
    cv::Mat brightened;
    cv::Mat frame;
    //cv::Mat extraSharpnessKernel = (cv::Mat_<float>(3,3) << -1,-1,-1,-1,9,-1,-1,-1,-1); /*Delete*/
    //cv::filter2D(irMAT,brightened,-1,extraSharpnessKernel); /*Delete*/
    //cv::equalizeHist(irMAT, brightened);
    clahe->apply(irMAT,brightened);
    //frame = irMAT;
    frame = brightened;
    
    
    //Image Scaling Block
    cv::Mat scaledFrame;
    cv::resize(croppedBrightened, scaledFrame, cv::Size(), 10, 10, cv::INTER_CUBIC);
    cv::Mat scaledDetectionMat,scaledAxesMat;
    cv::cvtColor(scaledFrame,scaledDetectionMat,cv::COLOR_GRAY2BGR);
    cv::cvtColor(scaledFrame,scaledAxesMat,cv::COLOR_GRAY2BGR);
    
    //Sharpness Block
    cv::Mat sharpenedFrame;
    //cv::Mat sharpnessKernel = (cv::Mat_<float>(3,3) << -1,-1,-1,-1,9,-1,-1,-1,-1); //Small kernel
    cv::Mat sharpnessKernel = (cv::Mat_<float>(5,5) << -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,25,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1); //Larger Kernel
    //cv::filter2D(scaledFrame,sharpenedFrame,-1,sharpnessKernel);
    cv::filter2D(scaledFrame,sharpenedFrame,-1,sharpnessKernel);
    /*
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
    /*
    cv::aruco::drawDetectedMarkers(sharpenedDetectionMat, sharpenedRejectedCandidates, sharpenedMarkerIds);
    cv::aruco::drawDetectedMarkers(detectionMat, rejectedCandidates, markerIds);
    for(int i = 0; i < sharpenedRejectedCandidates.size(); i++){
    	std::cout << sharpenedRejectedCandidates[0] << std::endl;
    }*/ 
    
    //Visualization Block
    cv::imshow("Image", irMAT);
    cv::imshow("Brightened", brightened);
    //cv::imshow("Detection" , detectionMat);
    //cv::imshow("Axes" , axesMat);
    cv::imshow("Scaled", scaledFrame);
    //cv::imshow("Scaled Detection" , scaledDetectionMat);
    //cv::imshow("Scaled Axes" , scaledAxesMat);
    cv::imshow("Sharpened", sharpenedFrame);
    //cv::imshow("Sharpened Detection", sharpenedDetectionMat);
    cv::imshow("Depth Thresholded", depthMatOriginal); //Display the original depth image with fitted circle and center point
    cv::imshow("Gaussian Blurred Image 9x9 kernel",gaussianBlur);
        
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
    }
    cv::destroyAllWindows();
    cv::waitKey(1);
    frames.clear();      
}
