#ifndef LocPredictor_h
#define LocPredictor_h

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include <cmath>
#include "V2.h"

class LocPredictor{
public:
    LocPredictor();
    LocPredictor(int width, int height);
    bool isSecondMeas();
    cv::Rect ROIPrediction(float timeStamp);
    coord2D pixelPosPredictor(float timeStamp);
    float depthPred(float timeStamp);
    void updateROIPredictor(coord2D currPos, float timeStamp, int currRadius);
    cv::Rect createROI(cv::Vec3f ballCircle, float error);
    float radiusPred(coord2D ballLocDepth, const struct rs2_intrinsics * intrin);
    // void findEdge(float pixel[2], float changeX, float changeY, float point[3]);
    
private:
    coord2D prevPos;
    coord2D velo;
    float prevTimeStamp = 0;
    int secondMeas = 0;
    int ROISize = 0;
    cv::Rect ROI;
    cv::Rect imageSize;
};

#endif /* LocPredictor_h */
