#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include <cmath>
#include "ThresholdFilter.h"
#include "D400.h"
#include "V2.h"

ThresholdFilter::ThresholdFilter(D400 &cam) : camera(cam){
}

//This filters out any depth that extends past (background depth - threshDist)
//NOTE TO SELF; Should change copying method as it's expensive
cv::Mat ThresholdFilter::filter(cv::Mat depthMat, float threshDist = 0.6096, float maxThreshDist = 3.048){
    cv::Mat retMat = depthMat; /*depthMat.clone();*/
    for (int i=0; i < depthMat.rows; i++){
        for(int j=0; j < depthMat.cols; j++){
            float threshMatDepth = thresholdMat.at<float>(i,j);
            float currMatDepth = depthMat.at<float>(i,j);
            if (threshMatDepth
                && currMatDepth > (threshMatDepth - threshDist)){
                
                retMat.at<float>(i,j) = 0;
            }
            else if (currMatDepth > maxThreshDist){
                retMat.at<float>(i,j) = 0;
            }
        }
    }
    return retMat;
}

void ThresholdFilter::createThresholdFrame(){
    rs2::temporal_filter temp_filter(0.4, 20, 7);
    rs2::hole_filling_filter hole_fill_filter(2);
    
    camera.setDefaultSettings();
    camera.startStream();
    
    rs2::depth_frame thresholdFrame = temp_filter.process(camera.getFrame().get_depth_frame());
    for (int i = 0; i < 10; i++){
        thresholdFrame = hole_fill_filter.process(camera.getFrame().get_depth_frame());
        thresholdFrame = temp_filter.process(thresholdFrame);
    }
    camera.endStream();
    
    //thresholdFrame = hole_fill_filter.process(thresholdFrame);
    
    thresholdMat = rsDepthToCVMat(thresholdFrame);
}