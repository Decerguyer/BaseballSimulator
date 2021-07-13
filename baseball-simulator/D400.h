#ifndef D400_h
#define D400_h

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include "Camera.h"
#include "ImageData.h"
#include "V2.h"

class D400 : public Camera{
public:
    D400();
    D400(rs2::context &ctx);
    void setExposure(int exposureTime);
    void enableStreams(int width, int height, int fps);
    void enableDepthStream(int width, int height, int fps);
    void enableIRStream(int width, int height, int fps, int index = 1);
    void disableEmitter();
    void setDefaultSettings();  
    void setCalibrationSettings();
    void calibrate();
    std::deque<ImageData> recordImageData(int numFrames,int numThrow = 20);
    struct rs2_intrinsics getIntrinsics();
    rs2::frameset getFrame();
    void startStream(int numFrames = 20);
    void endStream(); 

    void write(cv::FileStorage& file) const;
    void read(const cv::FileNode& node);
    
private:
    rs2::config cfg;
    rs2::pipeline pipe;
    rs2::sensor depthSensor;
    bool isStreaming = false;
    struct rs2_intrinsics intrin;
    std::deque<rs2::frameset> recordRSFrames(int numFrames,int numThrow = 20);
    void throwFrames(int numFrames);

};

void write(cv::FileStorage& file, const std::string&, const D400& camera);
void read(const cv::FileNode& node, D400& camera, const D400& default_value);

#endif /* D400_h */