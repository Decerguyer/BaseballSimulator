// V2.h
//
// Baseball Simulator 'define' statements
//
//  Created by Yoni Arieh on 6/18/21.
//

#ifndef V2_h
#define V2_h

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include <cmath>

#define BASEBALL_RADIUS 0.036888
//Baseball radius in meters

#define NUM_CALIBRATION_FRAMES 100
//Number of frames per camera for chessboard calibration

#define SQUARE_SIZE 152.4
//Length of singular square on chessboard in mm


struct coord2D{
    float x;
    float y;
    float depth;
};

struct coord3D{
    float x;
    float y;
    float z;
};

cv::Mat rsDepthToCVMatExtra(rs2::depth_frame depthFrame){
    rs2::video_stream_profile prof = depthFrame.get_profile().as<rs2::video_stream_profile>();
    cv::Mat depth_metric_fp;
    cv::Mat depth_raw = cv::Mat(prof.height(), prof.width(), CV_16SC1, const_cast<void*>(depthFrame.get_data()));
    depth_raw.convertTo(depth_metric_fp, CV_32FC1);
    depth_metric_fp *= depthFrame.get_units();
    
    
    return depth_metric_fp;
}

#endif /* V2_h */




