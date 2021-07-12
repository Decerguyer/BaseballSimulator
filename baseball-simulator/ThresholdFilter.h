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

    void write(cv::FileStorage& file) const;
    void read(const cv::FileNode& node);

private:
    cv::Mat thresholdMat;
    D400 &camera;
    cv::Mat rsDepthToCVMatExtra(rs2::depth_frame depthFrame);
    
};

static void write(cv::FileStorage& file, const std::string&, const ThresholdFilter& threshFilter);
static void read(const cv::FileNode& node, ThresholdFilter& threshFilter);

#endif /* ThresholdFilter_h */
