#ifndef ThresholdFilter_h
#define ThresholdFilter_h

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include <cmath>
#include "D400.h"
#include "V2.h"

class ThresholdFilter{
public:
    ThresholdFilter(D400 &camera);
    cv::Mat filter(cv::Mat depthMat, float threshDist = 0.6096, float maxThreshDist = 3.048);
    void createThresholdFrame();

private:
    cv::Mat thresholdMat;
    D400 &camera;
};

cv::Mat rsDepthToCVMatExtra(rs2::depth_frame depthFrame){
    rs2::video_stream_profile prof = depthFrame.get_profile().as<rs2::video_stream_profile>();
    cv::Mat depth_metric_fp;
    cv::Mat depth_raw = cv::Mat(prof.height(), prof.width(), CV_16SC1, const_cast<void*>(depthFrame.get_data()));
    depth_raw.convertTo(depth_metric_fp, CV_32FC1);
    depth_metric_fp *= depthFrame.get_units();
    
    
    return depth_metric_fp;
}

#endif /* ThresholdFilter_h */
