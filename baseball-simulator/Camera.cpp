#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <climits>
#include <vector>
#include <deque>
#include "Camera.h"
#include "V2.h"
#include "ImageData.h"

Camera::Camera(){
}

cv::Mat Camera::getRotationMatrix(){
    return rotationMatrix;
}

cv::Mat Camera::getTranslationMatrix(){
    return translationMatrix;
}

std::vector<float> Camera::transformPoint(std::vector<float> untransformedVec){
    cv::Mat untransformedPoint = convertPosVecMat(untransformedVec);
    cv::Mat untransformedPointmm = (cv::Mat_<double>(3,1) << untransformedPoint.at<double>(0)*1000,untransformedPoint.at<double>(1)*1000,untransformedPoint.at<double>(2)*1000); //Converting m to mm
    cv::Mat transformedPointmm=rotationMatrix*(untransformedPointmm-translationMatrix);
    cv::Mat transformedPoint = (cv::Mat_<double>(3,1) << transformedPointmm.at<double>(0)/1000,transformedPointmm.at<double>(1)/1000,transformedPointmm.at<double>(2)/1000); //Converting m to mm
    std::vector<float> transformedVec= convertPosMatVec(transformedPoint);
    return transformedVec;
}

//Only rotates doesn't translate
std::vector<float> Camera::transformError(std::vector<float> untransformedVec){
    cv::Mat untransformedPoint = convertPosVecMat(untransformedVec);
    cv::Mat untransformedPointmm = (cv::Mat_<double>(3,1) << untransformedPoint.at<double>(0)*1000,untransformedPoint.at<double>(1)*1000,untransformedPoint.at<double>(2)*1000); //Converting m to mm
    cv::Mat transformedPointmm=rotationMatrix*(untransformedPointmm);
    cv::Mat transformedPoint = (cv::Mat_<double>(3,1) << transformedPointmm.at<double>(0)/1000,transformedPointmm.at<double>(1)/1000,transformedPointmm.at<double>(2)/1000); //Converting m to mm
    std::vector<float> transformedVec = convertPosMatVec(transformedPoint);
    return transformedPoint;
}

void Camera::writeMat(cv::Mat mat, std::string fileName){
    fileName.append(".xml");
    cv::FileStorage file(fileName, cv::FileStorage::WRITE);

    file << "saved_matrix" << mat;
    file.release();
}

cv::Mat Camera::readMat(std::string fileName){
    fileName.append(".xml");
    cv::FileStorage file(fileName, cv::FileStorage::READ);
    cv::Mat retMatrix;
    file["saved_matrix"] >> retMatrix;
    file.release();
    return retMatrix;
}

void Camera::createCalibration(std::deque<ImageData> &frames){
    
    std::vector<cv::Point3f> objp; // Defining the world coordinates for 3D points
    for(int i=0; i<CHECKERBOARD[1]; i++){
        for(int j=0; j<CHECKERBOARD[0]; j++)
            objp.push_back(cv::Point3f(j*SQUARE_SIZE,i*SQUARE_SIZE,0));
    }
    
    // vector to store the pixel coordinates of detected checker board corners
    std::vector<cv::Point2f> corner_pts;
    
    bool success;
    int errorCnt = 0;
    
    for(int i =0; i<(int)frames.size(); i++){    //Iterates through the vector of framesets
        cv::Mat irMAT = frames[i].getIRMat();
        // Finding checker board corners
        // If desired number of corners are found in the image then success = true
        cv::Mat frame = irMAT;
        cv::Mat axesMat; //OPTIONAL
        cv::cvtColor(irMAT,axesMat,cv::COLOR_GRAY2BGR); //OPTIONAL
        
        success = cv::findChessboardCorners(frame, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts);
        // cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE
        
        /*
            * If desired number of corner are detected,
            * we refine the pixel coordinates and display
            * them on the images of checker board
        */   
        if(success){
                cv::TermCriteria criteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 0.001);
                // refining pixel coordinates for given 2d points.
                cv::cornerSubPix(frame,corner_pts,cv::Size(11,11), cv::Size(-1,-1),criteria);    
                
                //Calibration Mathematics 
                corner_pts = sortCornerPoints(corner_pts);

                verification_corner_pts = corner_pts;

                // Displaying the detected corner points on the checker board
                cv::drawChessboardCorners(frame, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, success); //OPTIONAL
                
                cv::Mat R,T; //Output Rotation and Translation Vectors
                cv::solvePnP(objp,corner_pts,cameraMatrix,distCoeffs,R,T,0,0); //Last 0 represents CV_ITERATIVE method
                
                cv::Mat rotation, rotationInverse;
                cv::Rodrigues(R,rotation);
                cv::invert(rotation,rotationInverse);
                rotationMatrix += rotationInverse;
                translationMatrix += T;
        }
        
        else
            errorCnt++;

//***********************************************************TERMINATION STATEMENT MUST BE CHANGED FROM DEBUG TO PRODUCTION*******************************************************
        if(errorCnt > 30){ //Termination condition, shows picture anyway to see why camera couldn't find chessboard
            std::cout<<"No Chessboard Found"<<std::endl;
            cv::imshow("Image",frame);
            cv::waitKey(0);
            cv::destroyAllWindows();
            cv::waitKey(1);
            frames.clear();
            return;
        }
    }
    //Normalizing the result using basic averages
    rotationMatrix /= (NUM_CALIBRATION_FRAMES - errorCnt);
    translationMatrix /= (NUM_CALIBRATION_FRAMES - errorCnt);
     //Delete Block Below
    cv::imshow("Image",frames[0].getIRMat());
    cv::waitKey(0);
    cv::destroyAllWindows();
    cv::waitKey(1);

}

std::vector<cv::Point2f> Camera::sortCornerPoints(std::vector<cv::Point2f> cornerPoints) {
    std::vector<cv::Point2f> finalSortedCorners;
    for(int j = 0; j < rows; j++){
        std::vector<cv::Point2f> ySortedPoints;
        for(int i = 0; i < columns; i++) {
            int smallestYVal = INT_MAX;
            int smallestYInd = 0;
            for(int k = 0; k < cornerPoints.size(); k++) {
                if(cornerPoints[k].y < smallestYVal) {
                    smallestYInd = k;
                    smallestYVal = cornerPoints[k].y;
                }
            }
            ySortedPoints.push_back(cornerPoints[smallestYInd]);
            cornerPoints.erase(cornerPoints.begin() + smallestYInd);
        }
        for(int i = 0; i < columns; i++){
            int smallestXVal = INT_MAX;
            int smallestXInd = 0;
            for(int k = 0; k < ySortedPoints.size(); k++) {
                if(ySortedPoints[k].x < smallestXVal) {
                    smallestXInd = k;
                    smallestXVal = ySortedPoints[k].x;
                }
            }
            finalSortedCorners.push_back(ySortedPoints[smallestXInd]);
            ySortedPoints.erase(ySortedPoints.begin() + smallestXInd);
        }
    }
    return finalSortedCorners;
}

cv::Mat Camera::convertPosVecMat(std::vector<float> unconverted){
    cv::Mat converted = (cv::Mat_<double>(3,1) <<double(unconverted[0]),double(unconverted[1]),double(unconverted[2]));
    return converted;
}


std::vector<float> Camera::convertPosMatVec(cv::Mat unconverted){
    std::vector<float> converted;
    converted.push_back(float(unconverted.at<double>(0)));
    converted.push_back(float(unconverted.at<double>(1)));
    converted.push_back(float(unconverted.at<double>(2)));
    return converted;
}

void Camera::write(cv::FileStorage& file) const  
{
    file << "{" 
    << "rotationMatrix" << this->rotationMatrix 
    << "translationMatrix" << this->translationMatrix 
    << "cameraMatrix" << this->cameraMatrix
    << "}";
}
void Camera::read(const cv::FileNode& node)
{
    node["rotationMatrix"] >> this->rotationMatrix;
    node["translationMatrix"] >> this->translationMatrix;
    node["cameraMatrix"] >> this->cameraMatrix;
}

void write(cv::FileStorage& file, const std::string&, const Camera& camera)
{
    camera.write(file);
}

void read(const cv::FileNode& node, Camera& camera, const Camera& default_value = Camera()){
    if(node.empty())
        std::cout << "No Data found in file\n";
    else
        camera.read(node);
}