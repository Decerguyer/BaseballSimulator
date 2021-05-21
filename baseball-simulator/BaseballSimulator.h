//
//  BaseballSimulator.h
//  librealsense2
//
//  Created by Jordan Mayor on 1/8/21.
//

#ifndef BaseballSimulator_h
#define BaseballSimulator_h

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include <cmath>

#define BASEBALL_RADIUS 0.0365


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

#endif /* BaseballSimulator_h */




