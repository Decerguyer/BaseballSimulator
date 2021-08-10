#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include "Camera.h"
#include "Tracker.h"
#include "ImageData.h"
#include "D400.h"
#include "V2.h"

D400::D400(){

}

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
    checkCalibration(calibrationFrames);
    setDefaultSettings();
}

void D400::checkCalibration(std::deque<ImageData> &frames){
    std::cout << "\nTesting Calibration... " << std::endl;
    //std::cout << verification_corner_pts << std::endl;
    std::vector<float> position;
    float depth;
    
    std::vector<coord2D> coord2DVec;
    coord2D TL;
    depth = frames[0].getDepthAt(verification_corner_pts[0].x, verification_corner_pts[0].y);
    TL.x = verification_corner_pts[0].x;
    TL.y = verification_corner_pts[0].y;
    TL.depth = depth;
    coord2DVec.push_back(TL);
    
    coord2D TR;
    depth = frames[0].getDepthAt(verification_corner_pts[2].x, verification_corner_pts[2].y);
    TR.x = verification_corner_pts[2].x;
    TR.y = verification_corner_pts[2].y;
    TR.depth = depth;
    coord2DVec.push_back(TR);
    
    coord2D BL;
    depth = frames[0].getDepthAt(verification_corner_pts[6].x, verification_corner_pts[6].y);
    BL.x = verification_corner_pts[6].x;
    BL.y = verification_corner_pts[6].y;
    BL.depth = depth;
    coord2DVec.push_back(BL);
    
    coord2D BR;	
    depth = frames[0].getDepthAt(verification_corner_pts[8].x, verification_corner_pts[8].y);
    BR.x = verification_corner_pts[8].x;
    BR.y = verification_corner_pts[8].y;
    BR.depth = depth;
    coord2DVec.push_back(BR);

    std::vector<std::vector<float>> positions3D = Tracker::convertTo3D(coord2DVec, intrin);
    std::vector<std::vector<float>> transformedPositions;
    
    for(int l = 0; l < positions3D.size(); l++){
        transformedPositions.push_back(transformPoint(positions3D[l]));
        std::cout << "x: " << transformedPositions[l][0] << " y: " << transformedPositions[l][1] << " z: " << transformedPositions[l][2] << std::endl;
    }

    std::cout << "\n\nOffset TL: \n" << "x: " << transformedPositions[0][0]-0 << " y: " << transformedPositions[0][1]-0 << " z: " << transformedPositions[0][2]-0 << std::endl;
    std::cout << "\n\nOffset TR: \n" << "x: " << transformedPositions[1][0]-(2*SQUARE_SIZE)/1000 << " y: " << transformedPositions[1][1]-0 << " z: " << transformedPositions[1][2]-0 << std::endl;
    std::cout << "\n\nOffset BL: \n" << "x: " << transformedPositions[2][0]-0 << " y: " << transformedPositions[2][1]-(2*SQUARE_SIZE)/1000 << " z: " << transformedPositions[2][2]-0 << std::endl;
    std::cout << "\n\nOffset BR: \n" << "x: " << transformedPositions[3][0]-(2*SQUARE_SIZE)/1000 << " y: " << transformedPositions[3][1]-(2*SQUARE_SIZE)/1000 << " z: " << transformedPositions[3][2]-0 << std::endl;
    std::cout << "\n" << std::endl;

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
    file << "{" << "Camera";
    Camera::write(file);
    file << "intrinWidth" << this->intrin.width
    << "intrinHeight" << this->intrin.height
    << "intrinPpx" << this->intrin.ppx
    << "intrinPpy" << this->intrin.ppy
    << "intrinFx" << this->intrin.fx
    << "intrinFy" << this->intrin.fy
    << "intrinModel" << this->intrin.model
    << "intrinCoeffs0" << this->intrin.coeffs[0]
    << "intrinCoeffs1" << this->intrin.coeffs[1]
    << "intrinCoeffs2" << this->intrin.coeffs[2]
    << "intrinCoeffs3" << this->intrin.coeffs[3]
    << "intrinCoeffs4" << this->intrin.coeffs[4]
    << "}";
}
void D400::read(const cv::FileNode& node)
{
    Camera::read(node["Camera"]);
    this->intrin.width = (int)node["intrinWidth"];
    this->intrin.height = (int)node["intrinHeight"];
    this->intrin.ppx = (float)node["intrinPpx"];
    this->intrin.ppy = (float)node["intrinPpy"];
    this->intrin.fx = (float)node["intrinFx"];
    this->intrin.fy = (float)node["intrinFy"];
    this->intrin.model = static_cast<rs2_distortion>((int)node["intrinModel"]);

    //Redo...
    this->intrin.coeffs[0] = (float)node["intrinCoeffs0"];
    this->intrin.coeffs[1] = (float)node["intrinCoeffs1"];
    this->intrin.coeffs[2] = (float)node["intrinCoeffs2"];
    this->intrin.coeffs[3] = (float)node["intrinCoeffs3"];
    this->intrin.coeffs[4] = (float)node["intrinCoeffs4"];
}

void write(cv::FileStorage& file, const std::string&, const D400& camera)
{
    camera.write(file);
}

void read(const cv::FileNode& node, D400& camera, const D400& default_value = D400()){
    if(node.empty())
        std::cout << "No Data found in file\n";
    else
        camera.read(node);
}
