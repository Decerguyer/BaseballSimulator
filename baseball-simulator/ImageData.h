#ifndef ImageData_h
#define ImageData_h

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include <cmath>

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
        depthVisMat = depthToVisual(depthMat);
        timeStamp = fSet.get_depth_frame().get_frame_metadata(RS2_FRAME_METADATA_SENSOR_TIMESTAMP);
        frameNumber = fSet.get_depth_frame().get_frame_number();
        
        /*
         This can be optimized later on where the full images arent needed
         and we just immediately use the crop.
        */
        depthMatCropped = depthMat;
        irMatCropped = irMat;
        depthVisMatCropped = depthVisMat;
    }
    
    cv::Mat getDepthMat(){
        return depthMat;
    }
    
    cv::Mat getIRMat(){
        return irMat;
    }
    
    cv::Mat getDepthVisMat(){
        return depthVisMat;
    }
    
    cv::Mat getDepthMatCropped(){
        return depthMatCropped;
    }
    
    cv::Mat getIRMatCropped(){
        return irMatCropped;
    }
    
    cv::Mat getDepthVisMatCropped(){
        return depthVisMatCropped;
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
    
    
    cv::Mat depthMat;
    cv::Mat irMat;
    cv::Mat depthVisMat;
    
    cv::Mat depthMatCropped;
    cv::Mat irMatCropped;
    cv::Mat depthVisMatCropped;
    
    cv::Vec3f depthVisBallLoc = {0, 0 , 0};
    cv::Vec3f IRBallLoc = {0, 0, 0};

    cv::Mat depthToVisual(cv::Mat depth){
    cv::Mat depthVis;
    depth.convertTo(depthVis, CV_8UC1, 255,0); //Convert the copy to 8 bit grey-scale format
    return depthVis;
    }

    
private:
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

#endif /* ImageData_h */
