//  Camera.h
//
//  Generic Camera Class with calibration methods
//
//  Not to be used independently. Should be used through a specific camera
//  ex. D400 that INHERITS from the camera class
//
//  Created by Yoni Arieh on 2/9/21.
//

#ifndef Camera_h
#define Camera_h

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <climits>
#include "ImageData.h"
#include "V2.h"
//#include "BaseballSimulator.h" 


class Camera{
public:
    Camera();
    cv::Mat getRotationMatrix();
    cv::Mat getTranslationMatrix();
    std::vector<float> transformPoint(std::vector<float> untransformedVec);
    std::vector<float> transformError(std::vector<float> untransformedVec);
    void writeMat(cv::Mat mat, std::string fileName);
    cv::Mat readMat(std::string fileName);

    void write(FileStorage& file) const;
    void read(const FileNode& node);
    
protected:
    cv::Mat cameraMatrix; //This is set by the subclass camera (For D400 this happens in startStream)
    cv::Mat distCoeffs = (cv::Mat_<float>(1,5) << 0,0,0,0,0); //Distortion coeff, should be 0 for all usable realsense cameras
    void createCalibration(std::deque<ImageData> &frames);
    
private:
    cv::Mat rotationMatrix = (cv::Mat_<double>(3,3) << 0,0,0,0,0,0,0,0,0);
    cv::Mat translationMatrix = (cv::Mat_<double>(3,1) << 0,0,0);
    int rows = 3;
    int columns = 3;
    int CHECKERBOARD[2] = {columns, rows};
    std::vector<cv::Point2f> sortCornerPoints(std::vector<cv::Point2f> cornerPoints); 
    cv::Mat convertPosVecMat(std::vector<float> unconverted);
    std::vector<float> convertPosMatVec(cv::Mat unconverted);

};

static void write(FileStorage& file, const std::string&, const Camera& camera);
static void read(const FileNode& node, Camera& camera);

#endif /* Camera_h */
