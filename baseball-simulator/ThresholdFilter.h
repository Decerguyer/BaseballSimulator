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
    cv::Mat rsDepthToCVMatExtra(rs2::depth_frame depthFrame);
    
};

#endif /* ThresholdFilter_h */
