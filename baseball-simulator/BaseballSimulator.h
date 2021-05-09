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
    
    
private:
    long long timeStamp;
    unsigned long long frameNumber;
    
    cv::Mat depthMat;
    cv::Mat irMat;
    cv::Mat depthVisMat;
    
    cv::Mat depthMatCropped;
    cv::Mat irMatCropped;
    cv::Mat depthVisMatCropped;
    
    cv::Vec3f depthVisBallLoc = {0, 0 , 0};
    cv::Vec3f IRBallLoc = {0, 0, 0};

    
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
    
    cv::Mat depthToVisual(cv::Mat depth){
        cv::Mat depthVis;
        depth.convertTo(depthVis, CV_8UC1, 255,0); //Convert the copy to 8 bit grey-scale format
        return depthVis;
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

class ThresholdFilter{
public:
    ThresholdFilter() {
        createThresholdFrame();
    }
    ThresholdFilter(Camera &cam) : camera{cam} {
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

struct coord2D{
    float x;
    float y;
    float depth;
};

struct coord3D{
    float x;
    float y;
    float z;
};

class ROIPredictor{
public:
    ROIPredictor(){
        ROI = cv::Rect(0, 0, 0, 0);
        imageSize = cv::Rect(0, 0, 0, 0);
        ROISize = 0;
    }
    
    ROIPredictor(int width, int height){
        ROI = cv::Rect(0, 0, width - 1, height - 1);
        imageSize = cv::Rect(0, 0, width - 1, height - 1);
        ROISize = 0;
    }
    
    cv::Rect ROIPrediction(float timeStamp){
        if (secondMeas == 0){
            ROI = imageSize;
        }
        else if (secondMeas == 1){
            ROI = cv::Rect(velo.x - ROISize*2, velo.y - ROISize*2, ROISize*4, ROISize*4);
        }
        else{
            coord2D predPos = pixelPosPredictor(timeStamp);
            ROI = cv::Rect(predPos.x - ROISize, predPos.y - ROISize, ROISize*2, ROISize*2);
        }
        
        return ROI & imageSize;
    }
    
    coord2D pixelPosPredictor(float timeStamp){
        coord2D predPos;
        predPos.x = prevPos.x + velo.x*timeStamp;
        predPos.y = prevPos.y + velo.y*timeStamp;
        predPos.depth = prevPos.depth + velo.depth*timeStamp;
        
        return predPos;
    }
    
    void updateROIPredictor(coord2D currPos, float timeStamp, int currRadius){
        velo.x = currPos.x - prevPos.x;
        velo.y = currPos.y - prevPos.y;
        velo.depth = currPos.depth - prevPos.depth;

        prevPos = currPos;
        ROISize = currRadius;
    }
    
    cv::Rect createROI(cv::Vec3f ballCircle, float error){
        float halfSize = ballCircle[2]*(error+1);
        cv::Rect tempROI(ballCircle[0]-halfSize, ballCircle[1]-halfSize, halfSize*2, halfSize*2);
        return tempROI & imageSize;
    }
    
    
private:
    coord2D prevPos;
    coord2D velo;
    float prevTimeStamp;
    int secondMeas = 0;
    
    int ROISize;
    cv::Rect ROI;
    cv::Rect imageSize;
};

class Tracker{
public:
    Tracker(int width, int height, struct rs2_intrinsics intrinsics){
        ROIPred = ROIPredictor(width, height);
        intrin = intrinsics;
    }
    
    coord2D findBallFromDepth(ImageData &imgData){
        imgData.depthVisMatCropped = cropDepthVis(imgData);
        cv::Vec3f ballCircle = depthVisHoughCircle(imgData, 0.25);
        
        cv::Point offset = ROIOffset(imgData.depthVisMatCropped);
        ballCircle[0] = ballCircle[0] + offset.x;
        ballCircle[1] = ballCircle[1] + offset.y;
        imgData.depthVisBallLoc = ballCircle;
        
        float meanDepth = 0;
        if (ballCircle[2]){
            meanDepth = findAveDepth(imgData, ballCircle);
        }
            
        coord2D ball2DCoord = {ballCircle[0], ballCircle[1], meanDepth};
        return ball2DCoord;
    }
    
    coord2D findBallFromIR(ImageData &imgData, cv::Vec3f ballCircleDepth, float meanDepth){
        imgData.irMatCropped = cropIR(imgData, ballCircleDepth, 0.25);
        cv::Vec3f ballCircleIR = irHoughCircle(imgData, 0.25);
        
        cv::Point offset = ROIOffset(imgData.irMatCropped);
        ballCircleIR[0] = ballCircleIR[0] + offset.x;
        ballCircleIR[1] = ballCircleIR[1] + offset.y;
        imgData.IRBallLoc = ballCircleIR;
        
        if (ballCircleIR[2]){
        imgData.irMatCropped = cropIR(imgData, ballCircleIR, 0.05);
        
        return {ballCircleIR[0], ballCircleIR[1], meanDepth};
        }
        else{
            return {0, 0, 0};
        }
    }
    

private:
    cv::Mat croppedDepthVis;

    int prevRadius = 0;
    
    struct rs2_intrinsics intrin;
    
    ROIPredictor ROIPred;
    
    
    //returns pixel x [0], pixel y [1], and the radius[2]
    cv::Vec3f depthVisHoughCircle(ImageData &imgData, int error){
        std::vector<cv::Vec3f> coords;
        
        //Should eventually refine parameters again
        cv::HoughCircles(imgData.depthVisMatCropped, coords, cv::HOUGH_GRADIENT, 1.6, 4000, 50, 5, prevRadius-prevRadius*error, prevRadius+prevRadius*error);
        
        if (!coords.empty()){
            return coords[0];
        }
        else{
            imgData.depthVisMatCropped = imgData.depthVisMat;
            cv::HoughCircles(imgData.depthVisMatCropped, coords, cv::HOUGH_GRADIENT, 1.6, 4000, 50, 5, prevRadius-prevRadius*error, prevRadius+prevRadius*error);
            if (!coords.empty()){
                return coords[0];
            }
            else{
                return cv::Vec3f(0, 0, 0);
            }
        }
    }
                           
    cv::Vec3f irHoughCircle(ImageData &imgData, int error){
        std::vector<cv::Vec3f> coords;
        
        //Should eventually refine parameters again
        cv::HoughCircles(imgData.irMatCropped, coords, cv::HOUGH_GRADIENT, 1.6, 4000, 50, 5, prevRadius-prevRadius*error, prevRadius+prevRadius*error);
        
        if (!coords.empty()){
            return coords[0];
        }
        else{
            imgData.irMatCropped = imgData.irMat;
            cv::HoughCircles(imgData.irMatCropped, coords, cv::HOUGH_GRADIENT, 1.6, 4000, 50, 5, prevRadius-prevRadius*error, prevRadius+prevRadius*error);
            if (!coords.empty()){
                return coords[0];
            }
            else{
                return cv::Vec3f(0, 0, 0);
            }
        }
    }
    
    cv::Mat cropDepthVis(ImageData &imgData){
        cv::Rect ROI = ROIPred.ROIPrediction(imgData.getTimeStamp());
        return imgData.depthVisMat(ROI);
    }
    
    cv::Mat cropIR(ImageData &imgData, cv::Vec3f ballCircle, float error){
        return imgData.irMat(ROIPred.createROI(ballCircle, error));
    }
    
    cv::Point ROIOffset(cv::Mat croppedImg){
        cv::Point offset;
        cv::Size wholesize;
        croppedImg.locateROI(wholesize, offset);
        return offset;
    }
    
    float findAveDepth(ImageData &imgData, cv::Vec3f circleCenter){
        float sum = 0;
        int numMeas = 0;
        for (int i = 0; i < 3; i++){
            for (int k = 0; k < 3; k++){
                if (imgData.getDepthAt(circleCenter[0] - 1 + i, circleCenter[1] - 1 + k))
                    numMeas++;
                sum += imgData.getDepthAt(circleCenter[0] - 1 + i, circleCenter[1] - 1 + k);
            }
        }
        float meanDepth = sum / numMeas;
        
        return meanDepth;
    }
};

class Visualizer{

public:
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
    
    void visualize(std::deque<ImageData> imgData, bool showDepth, bool showIR, bool drawCircles){
        for (int i = 0; i < (int)imgData.size(); (i >= 0 ? i++ : i = 0)){
            std::cout << "TimeStamp = " << imgData[i].getTimeStamp() << "; ";
            //std::cout << "Depth = " << ballDepthVec[i] << "; ";
            std::cout << "Frame = " << i + 1 << "/" << imgData.size() << "; ";
            std::cout << "Act Frame # = " << imgData[i].getFrameNumber() << std::endl;
            
            if (drawCircles){
                drawCircle(imgData[i].depthVisMat, imgData[i].depthVisBallLoc);
                drawCircle(imgData[i].irMat, imgData[i].IRBallLoc);
            }
            
            if (showDepth){
                imshow("Depth Visualized", imgData[i].depthVisMat);
                imshow("Depth Visualized ROI", imgData[i].depthVisMatCropped);
            }
            if (showIR){
                imshow("IR", imgData[i].irMat);
                imshow("IR Cropped", imgData[i].irMatCropped);
            }
            int key = cv::waitKey(0);
            if (key == 'n'){
                if (i == (int)imgData.size() - 1)
                    i--;
                else
                    continue;
            }
            else if (key == 27)
                break;
            else
                i -= 2;
        }
        cv::destroyAllWindows();
        cv::waitKey(1);
    }
};

#endif /* BaseballSimulator_h */




