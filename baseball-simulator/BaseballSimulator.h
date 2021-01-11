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


cv::Mat rsDepthToCVMat(rs2::depth_frame depthFrame){
    auto pf = depthFrame.get_profile().as<rs2::video_stream_profile>();

    cv::Mat depth_metric_fp;
    cv::Mat depth_raw = cv::Mat(pf.height(), pf.width(), CV_16SC1, const_cast<void*>(depthFrame.get_data()));
    depth_raw.convertTo(depth_metric_fp, CV_32FC1);
    depth_metric_fp *= depthFrame.get_units();
    
    
    return depth_metric_fp;
}


class Camera{
public:
    //Sets the exposure time of the depth sensor in microseconds
    Camera(){
        rs2::context ctx; // Create librealsense context for managing devices
        
        auto devList = ctx.query_devices(); // Get a snapshot of currently connected devices
        
        rs2::device dev = devList.front();
        
        depthSensor = dev.first<rs2::depth_sensor>();
    }
    
    void setExposure(int exposureTime){
        depthSensor.set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, false);
        depthSensor.set_option(RS2_OPTION_EXPOSURE, exposureTime);
    }
    
    void enableStreams(int width, int height, int fps){
        enableDepthStream(width, height, fps);
        enableIRStream(width, height, fps);
    }
    
    void enableDepthStream(int width, int height, int fps){
        cfg.enable_stream(RS2_STREAM_DEPTH, width, height, RS2_FORMAT_Z16, fps);
    }
    
    void enableIRStream(int width, int height, int fps, int index = 1){
        cfg.enable_stream(RS2_STREAM_INFRARED, index, width, height, RS2_FORMAT_Y8, fps);
    }
    
    void setDefaultSettings(){
    	depthSensor.set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, true);
	depthSensor.set_option(RS2_OPTION_GAIN, 16.f);
    	depthSensor.set_option(RS2_OPTION_EMITTER_ENABLED, true);
    	depthSensor.set_option(RS2_OPTION_EMITTER_ON_OFF, false);
    	enableDepthStream(848, 480, 90);
	enableIRStream(848, 480, 90);
    }
    
    /*Takes specified number of frames and
     returns a vector of the collected frames
     */
    std::vector<rs2::frameset> recordRSFrames(int numFrames){
        std::vector<rs2::frameset> frames;
        
        pipe.start(cfg);
        for (int i = 0; i < numFrames; i++){
            rs2::frameset fSet = pipe.wait_for_frames();
            
            fSet.keep();
            frames.push_back(fSet);
        }
        pipe.stop();
        return frames;
    }

    //returns the most recent frame
    rs2::frameset getFrame(){
        return pipe.wait_for_frames();
    }
    
    //starts a stream and throws out the first few frames
    void startStream(int numFrames = 0){
        pipe.start(cfg);
        if(numFrames > 0){throwFrames(numFrames);}
    }
    
    void endStream(){
        pipe.stop();
    }
    
    void throwFrames(int numFrames){
        for (int i = 0; i < numFrames; i++){
            getFrame();
        }
    }
    
    
private:
    rs2::config cfg;
    
    rs2::pipeline pipe;
    
    rs2::sensor depthSensor;
    
    bool isStreaming = false;
};

class ThresholdFilter{
public:
    ThresholdFilter() {
        createThresholdFrame();
    }
    ThresholdFilter(Camera cam) : camera{cam} {
        createThresholdFrame();
    }
    
    //This filters out any depth that extends past (background depth - threshDist)
    //NOTE TO SELF; Should change copying method as it's expensive
    cv::Mat filter(cv::Mat depthMat, float threshDist = 0.6096, float maxThreshDist = 3.048){
        cv::Mat retMat = depthMat.clone();
        for (int i=0; i < depthMat.rows; i++){
            for(int j=0; j < depthMat.cols; j++){
                float threshMatDepth = thresholdMat.at<float>(i,j);
                float currMatDepth = depthMat.at<float>(i,j);
                if (threshMatDepth
                    && currMatDepth > (threshMatDepth - threshDist)){
                    
                    retMat.at<float>(i,j) = 0;
                }
                else if (currMatDepth > maxThreshDist){
                    retMat.at<float>(i,j) = 0;
                }
            }
        }
        return retMat;
    }
    
    
private:
    Camera camera;
    cv::Mat thresholdMat;
    
    void createThresholdFrame(){
        rs2::temporal_filter temp_filter(0.4, 20, 7);
        rs2::hole_filling_filter hole_fill_filter(2);
        
        camera.setDefaultSettings();
        camera.startStream();
        
        rs2::depth_frame thresholdFrame = temp_filter.process(camera.getFrame().get_depth_frame());
        for (int i = 0; i < 10; i++){
            thresholdFrame = hole_fill_filter.process(camera.getFrame().get_depth_frame());
            thresholdFrame = temp_filter.process(thresholdFrame);
        }
        camera.endStream();
        
        //thresholdFrame = hole_fill_filter.process(thresholdFrame);
        
        thresholdMat = rsDepthToCVMat(thresholdFrame);
    }
};

//class Tracker{
//public:
//    Tracker(int width, int height){
//        ROI = cv::Rect(0, 0, width - 1, height - 1);
//        imageSize = cv::Rect(0, 0, width - 1, height - 1);
//
//    }
//private:
//    float bndBoxSize = 180;
//    
//    cv::Rect ROI;
//    cv::Rect imageSize;
//
//    
//};


#endif /* BaseballSimulator_h */
