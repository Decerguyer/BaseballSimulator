#ifndef Camera_h
#define Camera_h

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include "ImageData.h"

class Camera{
public:
    //Sets the exposure time of the depth sensor in microseconds
    Camera(){
        rs2::context ctx; // Create librealsense context for managing devices
        
        auto devList = ctx.query_devices(); // Get a snapshot of currently connected devices
        
        rs2::device dev = devList.front();
        
        depthSensor = dev.first<rs2::depth_sensor>();
        
        //intrin = depthSensor.get_stream_profiles().front().as<rs2::video_stream_profile>().get_intrinsics();
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
    
    void disableEmitter(){
        depthSensor.set_option(RS2_OPTION_EMITTER_ENABLED, false);
    }
    
    void setDefaultSettings(){
        depthSensor.set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, true);
        //depthSensor.set_option(RS2_OPTION_GAIN, 16.f);
        depthSensor.set_option(RS2_OPTION_EMITTER_ENABLED, true);
        depthSensor.set_option(RS2_OPTION_EMITTER_ON_OFF, false);
        enableDepthStream(848, 480, 90);
        enableIRStream(848, 480, 90);
    }
    
    /*Takes specified number of frames and
     returns a deque of the collected frames
     */
    std::deque<rs2::frameset> recordRSFrames(int numFrames,int numThrow = 20){
        std::deque<rs2::frameset> frames;
        
        startStream();
        throwFrames(numThrow);
        for (int i = 0; i < numFrames; i++){
            rs2::frameset fSet = pipe.wait_for_frames();
            
            fSet.keep();
            frames.push_back(fSet);
        }
        endStream();
        return frames;
    }
    
    std::deque<ImageData> recordImageData(int numFrames,int numThrow = 20){
        std::deque<rs2::frameset> recordedFrames = recordRSFrames(numFrames, numThrow);
        
        std::deque<ImageData> images;
        while(!recordedFrames.empty()){
            images.emplace_back(recordedFrames.front());
            recordedFrames.pop_front();
        }
        
        return images;
    }

    //returns the most recent frame
    rs2::frameset getFrame(){
        return pipe.wait_for_frames();
    }
    
    //starts a stream and throws out the first few frames
    void startStream(int numFrames = 20){
        pipe.start(cfg);
        intrin = pipe.get_active_profile().get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>().get_intrinsics();

        throwFrames(numFrames);
    }
    
    void endStream(){
        pipe.stop();
    }
    
    void throwFrames(int numFrames){
        for (int i = 0; i < numFrames; i++){
            getFrame();
        }
    }
    struct rs2_intrinsics getIntrinsics(){
        return intrin;
    }
    
private:
    rs2::config cfg;
    
    rs2::pipeline pipe;
    
    rs2::sensor depthSensor;
    
    bool isStreaming = false;
    
    struct rs2_intrinsics intrin;
};

#endif /* Camera_h */
