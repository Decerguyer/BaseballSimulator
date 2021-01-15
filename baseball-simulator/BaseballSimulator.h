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

cv::Mat rsDepthToCVMat(rs2::depth_frame depthFrame){
    auto pf = depthFrame.get_profile().as<rs2::video_stream_profile>();

    cv::Mat depth_metric_fp;
    cv::Mat depth_raw = cv::Mat(pf.height(), pf.width(), CV_16SC1, const_cast<void*>(depthFrame.get_data()));
    depth_raw.convertTo(depth_metric_fp, CV_32FC1);
    depth_metric_fp *= depthFrame.get_units();
    
    
    return depth_metric_fp;
}

class ImageData{
public:
    ImageData(rs2::frameset fSet){
        depthMat = rsDepthToCVMat(fSet.get_depth_frame());
        irMat = rsIRToCVMat(fSet.get_infrared_frame(1));
        timeStamp = fSet.get_depth_frame().get_frame_metadata(RS2_FRAME_METADATA_SENSOR_TIMESTAMP);
        frameNumber = fSet.get_depth_frame().get_frame_number();
    }
    
    cv::Mat getDepthMat(){
        return depthMat;
    }
    
    cv::Mat getIRMat(){
        return irMat;
    }
    
    float getDepthAt(int pixelX, int pixelY){
        return depthMat.at<float>(pixelY, pixelX);
    }
    
    long long getTimeStamp(){
        return timeStamp;
    }
    
    unsigned long long getFrameNumber(){
        return frameNumber;
    }
    
private:
    cv::Mat depthMat;
    cv::Mat irMat;
    
    long long timeStamp;
    unsigned long long frameNumber;

    
    cv::Mat rsDepthToCVMat(rs2::depth_frame depthFrame){
        rs2::video_stream_profile prof = depthFrame.get_profile().as<rs2::video_stream_profile>();
        cv::Mat depth_metric_fp;
        cv::Mat depth_raw = cv::Mat(prof.height(), prof.width(), CV_16SC1, const_cast<void*>(depthFrame.get_data()));
        depth_raw.convertTo(depth_metric_fp, CV_32FC1);
        depth_metric_fp *= depthFrame.get_units();
        
        
        return depth_metric_fp;
    }
    
    cv::Mat rsIRToCVMat(rs2::video_frame irFrame){
        rs2::video_stream_profile prof = irFrame.get_profile().as<rs2::video_stream_profile>();
        cv::Mat ir_mat = cv::Mat(prof.height(), prof.width(), CV_8UC1, (void*)irFrame.get_data());
        return ir_mat;
    }
    
    
};


class Camera{
public:
    //Sets the exposure time of the depth sensor in microseconds
    Camera(){
        rs2::context ctx; // Create librealsense context for managing devices
        
        auto devList = ctx.query_devices(); // Get a snapshot of currently connected devices
        
        rs2::device dev = devList.front();
        
        depthSensor = dev.first<rs2::depth_sensor>();
        
        intrin = depthSensor.get_stream_profiles().front().as<rs2::video_stream_profile>().get_intrinsics();
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
     returns a deque of the collected frames
     */
    std::deque<rs2::frameset> recordRSFrames(int numFrames,int numThrow = 20){
        std::deque<rs2::frameset> frames;
        
        pipe.start(cfg);
        throwFrames(numThrow);
        for (int i = 0; i < numFrames; i++){
            rs2::frameset fSet = pipe.wait_for_frames();
            
            fSet.keep();
            frames.push_back(fSet);
        }
        pipe.stop();
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


class Tracker{
public:
    Tracker(int width, int height){
        ROI = cv::Rect(0, 0, width - 1, height - 1);
        imageSize = cv::Rect(0, 0, width - 1, height - 1);
        inBoundsROI = ROI & imageSize;
    }
    
    //returns a Mat with a circle drawn on it
    cv::Mat drawCircle(cv::Mat image, cv::Vec3f coord){
        cv::Mat retMat = image.clone();
        cv::Point pnt(coord[0], coord[1]);
        
        //draw the outer circle //green
        circle(retMat, pnt, coord[2], cv::Scalar(0,255,0), 2, 8, 0 );
        //draw the center of the circle //red
        circle(retMat, pnt, 2 , cv::Scalar(0,0,255), 2, 8, 0 );
        
        return retMat;
    }
private:
    float bndBoxSize = 180;
    
    cv::Rect ROI;
    cv::Rect imageSize;
    cv::Rect inBoundsROI;
    
    int ballPixX;
    int ballPixY;
    
    //returns pixel x [0], pixel y [1], and the radius[0]
    cv::Vec3f findBall(cv::Mat image, double distance){
        std::vector<cv::Vec3f> coords;

        int radius = ballRadius(distance);
        
        //Should eventually refine parameters again
        cv::HoughCircles(image, coords, cv::HOUGH_GRADIENT, 1.6, 4000, 50, 5, (radius - radius*0.15), radius + radius*0.15);
        
        return coords[0];
    }
    
    // Returns the radius of the ball
    // Needs expansion
    int ballRadius(float ballDepth){
        int radius = (27*0.646)/ballDepth;
        
        if (radius > 25){
            radius = 25;
        }
    }
    
    void adjustROI(){
        
    }
    
};


#endif /* BaseballSimulator_h */
