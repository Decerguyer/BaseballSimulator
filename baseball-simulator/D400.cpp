#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include "Camera.h"
#include "ImageData.h"
#include "D400.h"
#include "V2.h"

D400::D400(rs2::context &ctx){
    //A context is a librealsense tool for managing devices
    auto devList = ctx.query_devices(); // Get a snapshot of currently connected devices
    rs2::device dev = devList.front();
    depthSensor = dev.first<rs2::depth_sensor>();
}

//Sets the exposure time of the depth sensor in microseconds
void D400::setExposure(int exposureTime){
    depthSensor.set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, false);
    depthSensor.set_option(RS2_OPTION_EXPOSURE, exposureTime);
}

void D400::enableStreams(int width, int height, int fps){
    enableDepthStream(width, height, fps);
    enableIRStream(width, height, fps);
}

void D400::enableDepthStream(int width, int height, int fps){
    cfg.enable_stream(RS2_STREAM_DEPTH, width, height, RS2_FORMAT_Z16, fps);
}

 void D400::enableIRStream(int width, int height, int fps, int index){
    cfg.enable_stream(RS2_STREAM_INFRARED, index, width, height, RS2_FORMAT_Y8, fps);
}

void D400::disableEmitter(){
    depthSensor.set_option(RS2_OPTION_EMITTER_ENABLED, false);
}

void D400::setDefaultSettings(){
    depthSensor.set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, true);
    //depthSensor.set_option(RS2_OPTION_GAIN, 16.f);
    depthSensor.set_option(RS2_OPTION_EMITTER_ENABLED, true);
    depthSensor.set_option(RS2_OPTION_EMITTER_ON_OFF, false);
    enableDepthStream(848, 480, 90);
    enableIRStream(848, 480, 90);
}

void D400::setCalibrationSettings(){
    setDefaultSettings();
    disableEmitter();
}

void D400::calibrate(){
    setCalibrationSettings();
    std::deque<ImageData> calibFrameSets = recordImageData(NUM_CALIBRATION_FRAMES);
    std::deque<ImageData> calibrationFrames;
    while(!calibFrameSets.empty()){
        calibrationFrames.emplace_back(calibFrameSets.front());
        calibFrameSets.pop_front();
    }
    Camera::createCalibration(calibrationFrames);
    setDefaultSettings();
}

std::deque<ImageData> D400::recordImageData(int numFrames,int numThrow){
    std::deque<rs2::frameset> recordedFrames = recordRSFrames(numFrames, numThrow);
    std::deque<ImageData> images;
    while(!recordedFrames.empty()){
        images.emplace_back(recordedFrames.front());
        recordedFrames.pop_front();
    }
    return images;
}

struct rs2_intrinsics D400::getIntrinsics(){
    //std::cout << "Fx: " << intrin.fx << std::endl << "Fy: " << intrin.fy << std::endl << "ppx: " << intrin.ppx << std::endl << "ppy: " << intrin.ppy << std::endl;
    return intrin;
}

/*Takes specified number of frames and
returns a deque of the collected frames
*/
std::deque<rs2::frameset> D400::recordRSFrames(int numFrames,int numThrow){
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

//returns the most recent frame
rs2::frameset D400::getFrame(){
    return pipe.wait_for_frames();
}

//starts a stream and throws out the first few frames
void D400::startStream(int numFrames){
    pipe.start(cfg);
    intrin = pipe.get_active_profile().get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>().get_intrinsics();
    Camera::cameraMatrix = (cv::Mat_<float>(3,3) << intrin.fx,0,intrin.ppx,0,intrin.fy,intrin.ppy,0,0,1);
    throwFrames(numFrames);
}

void D400::endStream(){
    pipe.stop();
}

void D400::throwFrames(int numFrames){
    for (int i = 0; i < numFrames; i++){
        getFrame();
    }
}

void D400::write(cv::FileStorage& file) const  
{
    // file << "{" 
    // << "rotationMatrix" << this->rotationMatrix 
    // << "translationMatrix" << this->translationMatrix 
    // << "cameraMatrix" << this->cameraMatrix

    // << "}";
}
void D400::read(const cv::FileNode& node)
{
    // node["rotationMatrix"] >> this->rotationMatrix;
    // node["translationMatrix"] >> this->translationMatrix;
    // node["cameraMatrix"] >> this->cameraMatrix;

    //Read overload for rs2_intrin later to avoid this unnecessary code
    this->intrin.width = node["intrinWidth"];
    this->intrin.height = node["intrinHeight"];
    this->intrin.ppx = node["intrinPpx"];
    this->intrin.ppy = node["intrinPpy"];
    this->intrin.fx = node["intrinFx"];
    this->intrin.fy = node["intrinFy"];
}

static void write(cv::FileStorage& file, const std::string&, const D400& camera)
{
    camera.write(file);
}

static void read(const cv::FileNode& node, D400& camera){
    if(node.empty())
        std::cout << "No Data found in file\n";
    else
        camera.read(node);
}