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

static void onMouse( int event, int x, int y, int f, void* ){
    if (event == cv::EVENT_LBUTTONDOWN)
        std::cout << x << " " << y << std::endl;
}

int main(){
    
    //Camera Initialization Block
    Camera cam;    //Create Camera Object
    cam.enableStreams(848, 480, 90);    //Specify stream parameters
    cam.setDefaultSettings();
    cam.disableEmitter();
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
    cv::Mat cameraMatrix = (cv::Mat_<float>(3,3) << intrin.fx,0,intrin.ppx,0,intrin.fy,intrin.ppy,0,0,1); //Camera intrinsics combine with RS2_Intrin to make automatic script instead of manual
    cv::Mat distCoeffs = (cv::Mat_<float>(1,5) << 0,0,0,0,0); //Distortion coeff, should be 0 for all usable realsense cameras
    
    std::vector<cv::Mat> vect;
    std::vector<long long> timeStamp;
    
    for(int i =0; i<(int)frames.size();(i >= 0 ? i++ : i = 0)){    //Iterates through the vector of framesets
    
    //Frame management block
    cv::Mat irMAT = frames[i].getIRMat();
   
    //Uncropped Image user feedback
    cv::Mat brightened;
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(40);
    clahe->apply(irMAT,brightened);
    
    //Manual Detection
    cv::namedWindow( "Brightened", cv::WINDOW_AUTOSIZE);
    cv::imshow("Brightened", brightened);
    cv::setMouseCallback("Brightened", onMouse, 0 );
    
    int block = cv::waitKey(0); 
    if(block == 'f'){
    	
    	timeStamp.push_back(frames[i].getTimeStamp());
    	
    	cv::Mat detectionMat;
    	cv::cvtColor(brightened,detectionMat,cv::COLOR_GRAY2BGR);
    	
    	std::vector<int> markerIds;
    	markerIds.clear();
    	std::vector<std::vector<cv::Point2f>> markerCorners;
    	markerCorners.clear();
    	
    	std::cout << "Enter the 4 corners in order" << std::endl;
    	cv::Point2f a,b,c,d;
    	std::cin >> a.x >> a.y >> b.x >> b.y >> c.x >> c.y >> d.x >> d.y;
    	
    	std::vector<cv::Point2f> corner;
    	corner.clear();
    	corner.push_back(a);
    	corner.push_back(b);
    	corner.push_back(c);
    	corner.push_back(d);
    	markerCorners.push_back(corner);
    	
    	std::cout << "Enter the ID" << std::endl;
    	int id;
    	std::cin >> id;
    	markerIds.push_back(id);
   	cv::aruco::drawDetectedMarkers(detectionMat, markerCorners, markerIds);	 	
    	cv::imshow("Detection" , detectionMat);
    	
    	
    	std::vector<cv::Vec3d> Rvecs, Tvecs;
    	cv::aruco::estimatePoseSingleMarkers(markerCorners, 23, cameraMatrix, distCoeffs, Rvecs, Tvecs);
    	cv::Mat axesMat;
    	cv::cvtColor(brightened,axesMat,cv::COLOR_GRAY2BGR);
    	cv::Mat rotation;
    	for (int i = 0; i < Rvecs.size(); ++i) {
    	auto Rvec = Rvecs[i];
    	auto Tvec = Tvecs[i];
    	cv::aruco::drawAxis(axesMat, cameraMatrix, distCoeffs, Rvec, Tvec, 11.5);
    	cv::imshow("Axes" , axesMat);
    	cv::Rodrigues(Rvec,rotation);
    	}
    	
    	cv::Mat e1 = (cv::Mat_<double>(3,1) << 1,0,0); //e1 
    	cv::Mat e2 = (cv::Mat_<double>(3,1) << 0,1,0); //e2 
    	cv::Mat e3 = (cv::Mat_<double>(3,1) << 0,0,1); //e3 
    	
    	cv::Mat refVect = rotation*e3;
    	std::cout<<rotation*e1<<std::endl;
    	std::cout<<rotation*e2<<std::endl;
    	std::cout<<rotation*e3<<std::endl;
    	
    	vect.push_back(refVect);
    	std::cout << "Vector Size: " << vect.size() << std::endl;
    	if(vect.size()==2){
    		std::cout<<"Spin Computation: " << std::endl;
    		
    		//Z Comp	
    		double xyVect1[2] = {vect[0].at<double>(0),vect[0].at<double>(1)}; 
    		double xyVect2[2] = {vect[1].at<double>(0),vect[1].at<double>(1)};
    		std::cout << "Vect 1: " << xyVect1[0] << " , " << xyVect1[1] << std::endl;
    		std::cout << "Vect 2: " << xyVect2[0] << " , " << xyVect2[1] << std::endl;
    		
    		double normFactorZ1 = 1/std::sqrt((xyVect1[0]*xyVect1[0]+xyVect1[1]*xyVect1[1]));
    		double normFactorZ2 = 1/std::sqrt((xyVect2[0]*xyVect2[0]+xyVect2[1]*xyVect2[1]));
    		xyVect1[0] = xyVect1[0]*normFactorZ1;
    		xyVect1[1] = xyVect1[1]*normFactorZ1;
    		xyVect2[0] = xyVect2[0]*normFactorZ2;
    		xyVect2[1] = xyVect2[1]*normFactorZ2;
    		
    		std::cout << "Vect 1 NORM: " << xyVect1[0] << " , " << xyVect1[1] << std::endl;
    		std::cout << "Vect 2 NORM: " << xyVect2[0] << " , " << xyVect2[1] << std::endl;
    		
    		float angleZ = std::acos(xyVect1[0]*xyVect2[0]+xyVect1[1]*xyVect2[1]);
    		float rotZ = 1000000*60*(std::acos(xyVect1[0]*xyVect2[0]+xyVect1[1]*xyVect2[1])/(2*3.141593))/(timeStamp[1]-timeStamp[0]);
    		std::cout << "Z rotation: " <<  rotZ << " RPM" << std::endl; 
    		std::cout << "Angle  Z" << angleZ << std::endl;
    		
    		//Y Comp
    		double xzVect1[2] = {vect[0].at<double>(0),vect[0].at<double>(2)}; 
    		double xzVect2[2] = {vect[1].at<double>(0),vect[1].at<double>(2)};
    		std::cout << "Vect 1: " << xzVect1[0] << " , " << xzVect1[1] << std::endl;
    		std::cout << "Vect 2: " << xzVect2[0] << " , " << xzVect2[1] << std::endl;
    		
    		double normFactorY1 = 1/std::sqrt((xzVect1[0]*xzVect1[0]+xzVect1[1]*xzVect1[1]));
    		double normFactorY2 = 1/std::sqrt((xzVect2[0]*xzVect2[0]+xzVect2[1]*xzVect2[1]));
    		xzVect1[0] = xzVect1[0]*normFactorY1;
    		xzVect1[1] = xzVect1[1]*normFactorY1;
    		xzVect2[0] = xzVect2[0]*normFactorY2;
    		xzVect2[1] = xzVect2[1]*normFactorY2;
    		
    		std::cout << "Vect 1 NORM: " << xzVect1[0] << " , " << xzVect1[1] << std::endl;
    		std::cout << "Vect 2 NORM: " << xzVect2[0] << " , " << xzVect2[1] << std::endl;
    		
    		float angleY = std::acos(xzVect1[0]*xzVect2[0]+xzVect1[1]*xzVect2[1]);
    		float rotY = 1000000*60*(std::acos(xzVect1[0]*xzVect2[0]+xzVect1[1]*xzVect2[1])/(2*3.141593))/(timeStamp[1]-timeStamp[0]);
    		std::cout << "Angle Y " << angleY << std::endl;
    		std::cout << "Y rotation: " <<  rotY << " RPM" << std::endl; 
    		
    		//X Comp
    		double yzVect1[2] = {vect[0].at<double>(1),vect[0].at<double>(2)}; 
    		double yzVect2[2] = {vect[1].at<double>(1),vect[1].at<double>(2)};
    		std::cout << "Vect 1: " << yzVect1[0] << " , " << yzVect1[1] << std::endl;
    		std::cout << "Vect 2: " << yzVect2[0] << " , " << yzVect2[1] << std::endl;
    		
    		double normFactorX1 = 1/std::sqrt((yzVect1[0]*yzVect1[0]+yzVect1[1]*yzVect1[1]));
    		double normFactorX2 = 1/std::sqrt((yzVect2[0]*yzVect2[0]+yzVect2[1]*yzVect2[1]));
    		yzVect1[0] = yzVect1[0]*normFactorX1;
    		yzVect1[1] = yzVect1[1]*normFactorX1;
    		yzVect2[0] = yzVect2[0]*normFactorX2;
    		yzVect2[1] = yzVect2[1]*normFactorX2;
    		
    		std::cout << "Vect 1 NORM: " << yzVect1[0] << " , " << yzVect1[1] << std::endl;
    		std::cout << "Vect 2 NORM: " << yzVect2[0] << " , " << yzVect2[1] << std::endl;
    		
    		float angleX = std::acos(yzVect1[0]*yzVect2[0]+yzVect1[1]*yzVect2[1]);
    		float rotX = 1000000*60*(std::acos(yzVect1[0]*yzVect2[0]+yzVect1[1]*yzVect2[1])/(2*3.141593))/(timeStamp[1]-timeStamp[0]);
    		std::cout << "Angle X " << angleX << std::endl;
    		std::cout << "X rotation: " <<  rotX << " RPM" << std::endl; 
    		
    	}
    	
    }
    //Visualization Block
    cv::imshow("Image", irMAT);
        
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
