#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include <cmath>
#include "ThresholdFilter.h"
#include "D400.h"
#include "V2.h"

ThresholdFilter::ThresholdFilter(D400 &cam) : camera(cam){
    createThresholdFrame();
}

//This filters out any depth that extends past (background depth - threshDist)
//NOTE TO SELF; Should change copying method as it's expensive
cv::Mat ThresholdFilter::filter(cv::Mat depthMat, float threshDist, float maxThreshDist){
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
    
    thresholdMat = rsDepthToCVMatExtra(thresholdFrame);
}

cv::Mat ThresholdFilter::rsDepthToCVMatExtra(rs2::depth_frame depthFrame){
    rs2::video_stream_profile prof = depthFrame.get_profile().as<rs2::video_stream_profile>();
    cv::Mat depth_metric_fp;
    cv::Mat depth_raw = cv::Mat(prof.height(), prof.width(), CV_16SC1, const_cast<void*>(depthFrame.get_data()));
    depth_raw.convertTo(depth_metric_fp, CV_32FC1);
    depth_metric_fp *= depthFrame.get_units();
    
    
    return depth_metric_fp;
}

void ThresholdFilter::write(cv::FileStorage& file) const  
{
    file << "{" 
    << "thresholdMat" << this->thresholdMat
    << "}";
}
void ThresholdFilter::read(const cv::FileNode& node)
{
    this->thresholdMat = node["thresholdMat"];
}

static void write(cv::FileStorage& file, const std::string&, const ThresholdFilter& threshFilter)
{
    threshFilter.write(file);
}

static void read(const cv::FileNode& node, ThresholdFilter& threshFilter){
    if(node.empty())
        std::cout << "No Data found in file\n";
    else
        threshFilter.read(node);
}
