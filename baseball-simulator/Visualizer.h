#ifndef Visualizer_h
#define Visualizer_h

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include <cmath>
#include "ImageData.h"
#include "V2.h"

class Visualizer{

public:
    Visualizer();
    cv::Mat drawCircle(cv::Mat image, cv::Vec3f coord);
    void visualize(std::deque<ImageData> imgData, bool showDepth, bool showIR, bool drawCircles);
    static void onMouse( int event, int x, int y, int f, void* );
};

#endif /* Visualizer_h */
