#ifndef ImageData_h
#define ImageData_h

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include <cmath>
#include "V2.h"
#include <string>

class ImageData{
public:
    ImageData(rs2::frameset fSet);
    ImageData();
    cv::Mat getDepthMat();
    cv::Mat getIRMat();
    cv::Mat getDepthVisMat();
    cv::Mat getDepthMatCropped();
    cv::Mat getIRMatCropped();
    cv::Mat getDepthVisMatCropped();
    float getDepthAt(int pixelX, int pixelY);
    long long getTimeStamp();
    unsigned long long getFrameNumber();
    cv::Mat depthMat;
    cv::Mat irMat;
    cv::Mat depthVisMat;
    cv::Mat depthMatCropped;
    cv::Mat irMatCropped;
    cv::Mat depthVisMatCropped;
    cv::Vec3f depthVisBallLoc = {0, 0 , 0};
    cv::Vec3f IRBallLoc = {0, 0, 0};
    cv::Mat depthToVisual(cv::Mat depth);

    void write(cv::FileStorage& file) const;
    void read(const cv::FileNode& node);

private:
    long long timeStamp;
    unsigned long long frameNumber;
    cv::Mat rsDepthToCVMat(rs2::depth_frame depthFrame);
    cv::Mat rsIRToCVMat(rs2::video_frame irFrame);

};

void write(cv::FileStorage& file, const std::string&, const ImageData& imgData);
void read(const cv::FileNode& node, ImageData& imgData, const ImageData& default_value);

//pass in an empty deque
void loadImageDataDeque(std::deque<ImageData>& images, cv::FileStorage& file);

void saveImageDataDeque(std::deque<ImageData>& images, cv::FileStorage& file);

#endif /* ImageData_h */
