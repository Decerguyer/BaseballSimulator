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
    
private:
    rs2::config cfg;
    rs2::pipeline pipe;
    rs2::sensor depthSensor;
    bool isStreaming = false;
    struct rs2_intrinsics intrin;
    std::deque<rs2::frameset> recordRSFrames(int numFrames,int numThrow = 20);
    rs2::frameset getFrame();
    void startStream(int numFrames = 20);
    void endStream(); 
    void throwFrames(int numFrames);

};

#endif /* D400_h */