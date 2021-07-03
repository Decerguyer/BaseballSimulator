#ifndef Tracker_h
#define Tracker_h

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include <cmath>
#include "LocPredictor.h"
#include "ThresholdFilter.h"
#include "ImageReconditioning.h"
#include "V2.h"

class Tracker{
public:
    Tracker(int width, int height, struct rs2_intrinsics intrinsics, ThresholdFilter &threshFilter);
    coord2D track(ImageData &imgData);
    coord2D findBallFromDepth(ImageData &imgData);
    coord2D findBallFromIR(ImageData &imgData, cv::Vec3f ballCircleDepth, coord2D ballCoordDepth);
    std::vector<std::vector<float>> convertTo3D(std::vector<coord2D> coord2DVec);

private:
    cv::Mat croppedDepthVis;
    int prevRadius = 0;
    struct rs2_intrinsics intrin;
    LocPredictor locPred;
    ThresholdFilter &threshFilter;
    ImageReconditioning imgRecondition;
    cv::Vec3f depthVisHoughCircle(ImageData &imgData, float error);
    cv::Vec3f irHoughCircle(ImageData &imgData, float radius, float error);
    cv::Mat cropFromPred(cv::Mat fullSizeMat, float timeStamp);
    cv::Mat cropIR(ImageData &imgData, cv::Vec3f ballCircle, float error);
    cv::Point ROIOffset(cv::Mat croppedImg);
    float findAveDepth(ImageData &imgData, cv::Vec3f circleCenter);
    
};

#endif /* Tracker_h */
