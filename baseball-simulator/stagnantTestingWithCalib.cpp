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
#include "CameraCalibration.h"

std::vector <cv::Vec3f> ball_detector(cv::Mat image, double distance){
    std::vector<cv::Vec3f> circles;

    int radius = (24*0.646)/distance;
    
    if (radius > 25){
        radius = 25;
    }
    
    cv::HoughCircles(image, circles, cv::HOUGH_GRADIENT, 1.6, 4000, 50, 5, (radius - radius*0.15), radius + radius*0.15);
    //std::cout<<circles.size()<<std::endl;
    return circles;
}

int main(){
    
    //Calibration Block
    CameraCalibration calibrationParameters;
    cv::Mat rotationMatrix,translationMatrix;
    rotationMatrix = calibrationParameters.getRotationMatrix();
    translationMatrix = calibrationParameters.getTranslationMatrix();
    std::cout << "Rotation Matrix: " << rotationMatrix << std::endl;
    std::cout << "Translation Matrix: " << translationMatrix << std::endl;
    
    //Camera Setup Block
    Camera cam;	//Create Camera Object
    cam.enableStreams(848, 480, 90);	//Specify stream parameters
    ThresholdFilter threshFilter;	//Create depth background for threshold/background-subraction
    
    //UI Command Line Block
    std::cout << "Enter the integer number of frames to measure: ";	//Give user time to set up the ball
    int num;
    std::cin >> num;
    
    //Recording Block
    std::vector<rs2::frameset> frames;
    frames = cam.recordRSFrames(num,30);	//(n,m) Record and save 'n' frames for measurements throw out first 'm' frames
    
    //Processing Block
    for(int i =0; i<(int)frames.size();(i >= 0 ? i++ : i = 0)){	//Iterates through the vector of framesets
        rs2::frameset fset = frames[i];	//Pulls the current frameset
        
        cv::Mat depthMatOriginal = rsDepthToCVMat(fset.get_depth_frame()); //Pull the depth frame from the frameset and convert it to cv::Mat format
        depthMatOriginal = threshFilter.filter(depthMatOriginal,0.3048,1);	//Send the depth frame through the threshold/background-subtraction filter
        
        cv::Mat depthMatCopy;	//Create a copy of the filtered depth frame
        depthMatCopy=depthMatOriginal;
	
	depthMatCopy.convertTo(depthMatCopy, CV_8UC1, 255,0); //Convert the copy to 8 bit grey-scale format
	
	float minThreshDist = 0.3048f;
	std::vector <cv::Vec3f> circles = ball_detector(depthMatCopy,minThreshDist); //Find the ball in the image
	
	float pixel[2] = {90, 90}; //Temporary center of the ball
        if (circles.size() >= 1){ //If the vector circles has a value (ball is found)
        	pixel[0] = (circles[0][0]) - (pixel[0] >= 90 ? 90 : pixel[0]) + pixel[0]; //Define the center of the ball column
        	pixel[1] = (circles[0][1]) - (pixel[1] >= 90 ? 90 : pixel[1]) + pixel[1]; //Define the center of the ball row
        	cv::Point pnt(pixel[0], pixel[1]); //Define the center point will be used to draw circle on top of the image 
        	std::cout<<"pixel column: " << pixel[0] << std::endl << "pixel row: " << pixel[1] << std::endl; //Output to user
        	
        	float distance = depthMatOriginal.at<float>(pixel[1], pixel[0]); //Retrieve distance of center from original depth image in cv MAT
        	
        	float tdPoint[3]; //Empty array of 3D coordinates 
        	auto intrin = cam.getIntrin(); //retreives the intrinsics of the camera necessary for deprojection
        	
        	//std::cout<<intrin<<std::endl;
		rs2_deproject_pixel_to_point(tdPoint, &intrin, pixel, distance); //Convert the distance and 2D pixel location into 3D position
        	
        	std::cout<<(tdPoint[0]/0.3048)*12<<std::endl; //x component
        	std::cout<<(tdPoint[1]/0.3048)*12<<std::endl; //y component
        	std::cout<<(tdPoint[2]/0.3048)*12<<std::endl; //z component
        	
        	std::cout<<std::endl<<std::endl<< "Transformed Point" << std::endl;
        	cv::Mat testPoint, transformedPoint;
    		testPoint = (cv::Mat_<double>(3,1) << tdPoint[0]*1000,tdPoint[1]*1000,tdPoint[2]*1000); //Converting m to mm
    		//testPoint = rotation*testPoint+T;
    		std::cout << "x: " << tdPoint[0] << " y: " << tdPoint[1] << " z: " << tdPoint[2] << std::endl;
    		transformedPoint=rotationMatrix*(testPoint-translationMatrix);
    		std::cout << "Transformed Point: " << transformedPoint << std::endl;
        	//Note that the origin is considered to be the left imager (facing outwards) x axis facing right, y axis facing down, z axis facing away
        	
        	cv::circle(depthMatOriginal, pnt, int(circles[0][2]), cv::Scalar(0,255,0), 2, 8, 0 ); //Draw a circle with the determined radius of the ball around ball's center point
        	cv::circle(depthMatOriginal, pnt, 2 , cv::Scalar(0,0,255), 2, 8, 0 ); //Draw a point marking the center of the ball
        }
        else
        	std::cout<<"No Circle Found" << std::endl;


        cv::imshow("Depth Thresholded", depthMatOriginal); //Display the original depth image with fitted circle and center point
        //cv::imshow("Depth Copy", depthMatCopy); //Display the original depth image with fitted circle and center point

        int key = cv::waitKey(0); //Blocks until user input
        if (key == 'n'){
        	if (i == (int)frames.size() - 1)
                    i--;
               else
                    continue;
        }
        else if (key == 27) //Pressing esc exits the program
               break;
        else
        	i -= 2;
    }
    cv::destroyAllWindows(); //Destroys the created window
    cv::waitKey(1); //Do not delete, necessary for proper destruction
    frames.clear(); //Empty vector
}

