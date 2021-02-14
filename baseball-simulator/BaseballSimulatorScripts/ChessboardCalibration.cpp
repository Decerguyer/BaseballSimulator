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

std::vector<cv::Point2f> sortCornerPoints(std::vector<cv::Point2f> cornerPoints,int rows,int columns) {
    std::vector<cv::Point2f> finalSortedCorners;
    for(int j = 0; j < rows; j++){
        std::vector<cv::Point2f> ySortedPoints;
            for(int i = 0; i < columns; i++) {
                int smallestYVal = 480;
                int smallestYInd = 0;
                for(int k = 0; k < cornerPoints.size(); k++) {
                    if(cornerPoints[k].y < smallestYVal) {
                        smallestYInd = k;
                        smallestYVal = cornerPoints[k].y;
                    }
                }
                ySortedPoints.push_back(cornerPoints[smallestYInd]);
                cornerPoints.erase(cornerPoints.begin() + smallestYInd);
            }
            for(int i = 0; i < columns; i++){
                int smallestXVal = 848;
                int smallestXInd = 0;
                for(int k = 0; k < ySortedPoints.size(); k++) {
                    if(ySortedPoints[k].x < smallestXVal) {
                        smallestXInd = k;
                        smallestXVal = ySortedPoints[k].x;
                    }
                }
                finalSortedCorners.push_back(ySortedPoints[smallestXInd]);
                ySortedPoints.erase(ySortedPoints.begin() + smallestXInd);
            }
        }
        return finalSortedCorners;
}


int main(){
    Camera cam;    //Create Camera Object
    cam.enableStreams(848, 480, 90);    //Specify stream parameters

    //ThresholdFilter threshFilter;    //Create depth background for threshold/background-subraction
    
    std::cout << "Enter the integer number of frames to measure: ";    //Give user time to set up the ball
    int num;
    std::cin >> num;
    
    std::deque<ImageData> frames;
    frames = cam.recordImageData(num,30);    //(n,m) Record and save 'n' frames for measurements throw out first 'm' frames
    
    auto intrin = cam.getIntrinsics(); //Retrieving camera intrinsics
    std::cout<<intrin.fx<<std::endl;
    std::cout<<intrin.fy<<std::endl;
    std::cout<<intrin.ppx<<std::endl;
    std::cout<<intrin.ppy<<std::endl;
    std::cout<<intrin.coeffs[0]<<intrin.coeffs[1]<<intrin.coeffs[2]<<intrin.coeffs[3]<<intrin.coeffs[4]<<std::endl;
  // Creating vector to store vectors of 3D points for each checkerboard image
  std::vector<std::vector<cv::Point3f> > objpoints;

  // Creating vector to store vectors of 2D points for each checkerboard image
  std::vector<std::vector<cv::Point2f> > imgpoints;
  
  int rows = 4;
  int columns = 5;
  //int CHECKERBOARD[2]={7,7}; //Small Checkerboard
  //float squareSize = 15.875; //in mm
  //int CHECKERBOARD[2]={4,4}; //Medium Checkerboard
  //float squareSize = 28.575; //in mm
  int CHECKERBOARD[2]={columns,rows}; //Large Checkerboard
  //float squareSize = 39.6875; //in mm
  //int CHECKERBOARD[2]={4,5}; //Printout Chckerboard
  float squareSize = 76.2; //in mm

  // Defining the world coordinates for 3D points
  std::vector<cv::Point3f> objp;
  for(int i=0; i<CHECKERBOARD[1]; i++)
  {
    for(int j=0; j<CHECKERBOARD[0]; j++)
      objp.push_back(cv::Point3f(j*squareSize,i*squareSize,0));
  }
  // vector to store the pixel coordinates of detected checker board corners
  std::vector<cv::Point2f> corner_pts;
  bool success;

    for(int i =0; i<(int)frames.size();(i >= 0 ? i++ : i = 0)){    //Iterates through the vector of framesets
        cv::Mat irMAT = frames[i].getIRMat();
        // Finding checker board corners
        // If desired number of corners are found in the image then success = true
        cv::Mat frame;
        frame = irMAT;
        cv::Mat axesMat;
        cv::cvtColor(irMAT,axesMat,cv::COLOR_GRAY2BGR);
        //cv::Mat axesMat = cv::Mat(cv::Size(848, 480), CV_8UC3, (void*)fsetCopy.get_infrared_frame(1).get_data());
        success = cv::findChessboardCorners(frame, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts);
        // cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE
        /*
         * If desired number of corner are detected,
         * we refine the pixel coordinates and display
         * them on the images of checker board
        */
        if(success)
        {
              cv::TermCriteria criteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 0.001);
      
              // refining pixel coordinates for given 2d points.
              cv::cornerSubPix(frame,corner_pts,cv::Size(11,11), cv::Size(-1,-1),criteria);
      
             // Displaying the detected corner points on the checker board
             cv::drawChessboardCorners(frame, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, success);
      
              objpoints.push_back(objp);
              imgpoints.push_back(corner_pts);
        }
        else{ //Termination condition, shows picture anyway to see why camera couldn't find chessboard
            std::cout<<"No Chessboard Found"<<std::endl;
            cv::imshow("Image",frame);
            cv::waitKey(0);
            cv::destroyAllWindows();
            cv::waitKey(1);
            frames.clear();
            return 0;
        }
        
        corner_pts = sortCornerPoints(corner_pts,rows,columns);
        //std::cout<<"Row: " << topRow << std::endl;
        //std::cout<<"Remaining: " << corner_pts << std::endl;
    
    std::cout<<"Frame #: " << i <<std::endl;
    
    cv::Mat cameraMatrix = (cv::Mat_<float>(3,3) << 422.353,0,419.783,0,422.353,239.109,0,0,1); //Camera intrinsics combine with RS2_Intrin to make automatic script instead of manual
    cv::Mat distCoeffs = (cv::Mat_<float>(1,5) << 0,0,0,0,0); //Distortion coeff, should be 0 for all usable realsense cameras
    cv::Mat R,T; //Output Rotation and Translation Vectors
    cv::solvePnP(objp,corner_pts,cameraMatrix,distCoeffs,R,T,0,0); //Last 0 represents CV_ITERATIVE method
    
    //cv::Mat cameraMatrix,distCoeffs,R,T;
        //cv::calibrateCamera(objpoints, imgpoints, cv::Size(frame.rows,frame.cols), cameraMatrix, distCoeffs, R, T);
        std::cout << "cameraMatrix : " << cameraMatrix << std::endl;
        std::cout << "distCoeffs : " << distCoeffs << std::endl;
        std::cout << "Rotation vector : " << R << std::endl;
        std::cout << "Translation vector : " << T << std::endl;
    
    std::vector<cv::Point3f> axisPoints;
    std::vector<cv::Point2f> imagePoints;
    axisPoints.push_back(cv::Point3f(3*squareSize,0,0));
    axisPoints.push_back(cv::Point3f(0,3*squareSize,0));
    axisPoints.push_back(cv::Point3f(0,0,3*squareSize));

    cv::projectPoints(axisPoints,R,T,cameraMatrix,distCoeffs,imagePoints);
    std::cout << "Image Points: " << imagePoints[0] << std::endl;
    std::cout << "Image Points: " << imagePoints[1] << std::endl;
    std::cout << "Image Points: " << imagePoints[2] << std::endl;
    cv::line(axesMat,corner_pts[0],imagePoints[0],cv::Scalar(255,0,0),2);
    cv::line(axesMat,corner_pts[0],imagePoints[1],cv::Scalar(0,255,0),2);
    cv::line(axesMat,corner_pts[0],imagePoints[2],cv::Scalar(0,0,255),2);
        
        cv::Mat rotation, rotationInverse;
        cv::Rodrigues(R,rotation);
        cv::invert(rotation,rotationInverse);
        
        //std::cout << "rotation matrix: " << rotation << std::endl;
        //std::cout << "inverse rotation matrix: " << rotationInverse << std::endl;
        //std::cout << "Corner Points Before: " << corner_pts << std::endl;
        cv::Mat depthMatOriginal = frames[i].getDepthMat(); //Pull the depth frame from the frameset and convert it to cv::Mat format
        //depthMatOriginal = threshFilter.filter(depthMatOriginal);    //Send the depth frame through the threshold/background-subtraction filter
        float tdPoint[3];
        float pixel[2];
        pixel[0]=corner_pts[0].x;
        pixel[1]=corner_pts[0].y;
        std::cout << "x Pixel: " << pixel[0] << " y Pixel: " << pixel[1] << std::endl;
        cv::Point pnt(pixel[0],pixel[1]); //Create Center point
        cv::circle(depthMatOriginal, pnt, 10, cv::Scalar(0,255,0), 2, 8, 0 ); //Draw a circle around chessboard origin
        float distance = depthMatOriginal.at<float>(pixel[1], pixel[0]); //Retrieve distance of center from original depth image in cv MAT
        //distance = 0.5575; //MANUAL SETTING DUE TO INVALID DEPTH POINTS
        std::cout << "distance: " <<distance << std::endl;
        rs2_deproject_pixel_to_point(tdPoint, &intrin, pixel, distance); //Convert the distance and 2D pixel location into 3D position
        
        cv::Mat testPoint, transformedPoint;
        testPoint = (cv::Mat_<double>(3,1) << tdPoint[0]*1000,tdPoint[1]*1000,tdPoint[2]*1000); //Converting m to mm
        //testPoint = rotation*testPoint+T;
        std::cout << "x: " << tdPoint[0] << " y: " << tdPoint[1] << " z: " << tdPoint[2] << std::endl;
        
        transformedPoint=rotationInverse*(testPoint-T);
        std::cout << "Transformed Point: " << transformedPoint << std::endl;
        
        //after
        //std::cout << "Corner Points After: " << corner_pts << std::endl;
        
        
        
        cv::imshow("Axes" , axesMat);
        cv::imshow("Image",frame);
     cv::imshow("Depth", depthMatOriginal);
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
