//
//  CameraCalibration.h
//  librealsense2
//
//  Created by Yoni Arieh on 2/9/21.
//

#ifndef CameraCalibration_h
#define CameraCalibration_h

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <climits>
//#include "BaseballSimulator.h" you need a camera object to run the calibration


class CameraCalibration{
public:
    CameraCalibration() {
        createCalibration();
    }
    CameraCalibration(Camera &cam) : camera{cam} {
        createCalibration();
    }

    cv::Mat getRotationMatrix(){
        return rotationMatrix;
    }
    cv::Mat getTranslationMatrix(){
        return translationMatrix;
    }
    cv::Mat convertPosVecMat(std::vector<float> unconverted){
        cv::Mat converted = (cv::Mat_<double>(3,1) << unconverted[0],unconverted[1],unconverted[2]);
        return converted;
    }
    cv::Mat convertPosMatVec(cv::Mat unconverted){
        std::vector<float> converted;
        converted.push_back(uncoverted.at<double>(0));
        converted.push_back(uncoverted.at<double>(1));
        converted.push_back(uncoverted.at<double>(2));
        return converted;
    }
    cv::Mat centerPointAdjust(cv::Mat point3D){
        cv::Mat adjustedPoint = geometricCenterAdjuster(point3D);
        return adjustedPoint;
    }
    cv::Mat transformPoint(cv::Mat adjustedPoint3D){
        cv::Mat transformedPoint = pointTransform(adjustedPoint3D);
        return transformedPoint;
    }
    void writeMat(cv::Mat mat, std::string fileName){
        fileName.append(".xml");
        cv::FileStorage file(fileName, cv::FileStorage::WRITE);

        file << "saved_matrix" << mat;
        file.release();
    }

    cv::Mat readMat(std::string fileName){
        fileName.append(".xml");
        cv::FileStorage file(fileName, cv::FileStorage::READ);
        cv::Mat retMatrix;
        file["saved_matrix"] >> retMatrix;
        file.release();
        return retMatrix;
    }
    
private:
    Camera camera;
    cv::Mat rotationMatrix = (cv::Mat_<double>(3,3) << 0,0,0,0,0,0,0,0,0);
    cv::Mat translationMatrix = (cv::Mat_<double>(3,1) << 0,0,0);
    const int numCalibrationFrames = 100; /* test to find optimal number*/
    //const int CHECKERBOARD[2]={4,4}; //Medium Chckerboard
    //const float squareSize = 28.575; //in mm
    //const int CHECKERBOARD[2]={3,3}; //Dimensions of CheckerPattern in corners
    //const float squareSize = 39.6875; //Square length in mm
    
    //Yoni Room Cedar
    /*
    int rows = 4;
    int columns = 5;
    int CHECKERBOARD[2]={columns,rows}; //Printout Chckerboard
    float squareSize = 76.2; //in mm
    */
    
    //Yoni Hallway Cedar
    ///*
    int rows = 3;
    int columns = 3;
    int CHECKERBOARD[2] = {columns, rows};
    float squareSize = 104.775;
    //*/
    void createCalibration(){
        camera.setDefaultSettings();
        camera.disableEmitter();
        std::deque<ImageData> frames;
        frames = camera.recordImageData(numCalibrationFrames,30); //(n,m) Record and save 'n' frames for measurements throw out first 'm' frames
        
        std::vector<std::vector<cv::Point3f> > objpoints;
        std::vector<std::vector<cv::Point2f> > imgpoints; // Creating vector to store vectors of 2D points for each checkerboard image
        
        
        std::vector<cv::Point3f> objp; // Defining the world coordinates for 3D points
        for(int i=0; i<CHECKERBOARD[1]; i++){
            for(int j=0; j<CHECKERBOARD[0]; j++)
                objp.push_back(cv::Point3f(j*squareSize,i*squareSize,0));
        }
        
        // vector to store the pixel coordinates of detected checker board corners
        std::vector<cv::Point2f> corner_pts;
        
        bool success;
        
        for(int i =0; i<(int)frames.size();(i >= 0 ? i++ : i = 0)){    //Iterates through the vector of framesets
            cv::Mat irMAT = frames[i].getIRMat();
            // Finding checker board corners
            // If desired number of corners are found in the image then success = true
            cv::Mat frame;
            frame = irMAT;
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
      
                 // Displaying the detected corner points on the checker board
                 cv::drawChessboardCorners(frame, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, success); //OPTIONAL
      
                  objpoints.push_back(objp);
                  imgpoints.push_back(corner_pts);
            }
            
            //***********************************************************ELSE STATEMENT MUST BE CHANGED FROM DEBUG TO PRODUCTION*******************************************************
            
            else{ //Termination condition, shows picture anyway to see why camera couldn't find chessboard
                std::cout<<"No Chessboard Found"<<std::endl;
                cv::imshow("Image",frame);
                cv::waitKey(0);
                cv::destroyAllWindows();
                cv::waitKey(1);
                frames.clear();
                //return 0;
            }
            corner_pts = sortCornerPoints(corner_pts,rows,columns);
            auto intrin = camera.getIntrinsics();
            //std::cout << "Fx: " << intrin.fx << std::endl << "Fy: " << intrin.fy << std::endl << "ppx: " << intrin.ppx << std::endl << "ppy: " << intrin.ppy << std::endl;
            cv::Mat cameraMatrix = (cv::Mat_<float>(3,3) << intrin.fx,0,intrin.ppx,0,intrin.fy,intrin.ppy,0,0,1);
            cv::Mat distCoeffs = (cv::Mat_<float>(1,5) << 0,0,0,0,0); //Distortion coeff, should be 0 for all usable realsense cameras
            cv::Mat R,T; //Output Rotation and Translation Vectors
            cv::solvePnP(objp,corner_pts,cameraMatrix,distCoeffs,R,T,0,0); //Last 0 represents CV_ITERATIVE method
            
            cv::Mat rotation, rotationInverse;
            cv::Rodrigues(R,rotation);
            cv::invert(rotation,rotationInverse);
        rotationMatrix += rotationInverse;
        translationMatrix += T;
        //Delete block below
        /*cv::imshow("Image",frame);
        cv::waitKey(0);
        cv::destroyAllWindows();
        cv::waitKey(1);
        */
    }
        rotationMatrix /= numCalibrationFrames;
        translationMatrix /= numCalibrationFrames;
        //Delete Block Below
        cv::imshow("Image",frames[0].getIRMat());
        cv::waitKey(0);
        cv::destroyAllWindows();
        cv::waitKey(1);
        
    }
    
    std::vector<cv::Point2f> sortCornerPoints(std::vector<cv::Point2f> cornerPoints,int rows,int columns) {
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
    cv::Mat geometricCenterAdjuster(cv::Mat point3D){
            double x = point3D.at<double>(0);
            double y = point3D.at<double>(1);
            double z = point3D.at<double>(2);

            double r = 0.036888; //Baseball radius in meters approximation

            double _x, _y, _z;

            _x = (x*std::sqrt(x*x + y*y + z*z + r))/std::sqrt(x*x + y*y + z*z); //Similar Triangles
            _y = (y*(x*x + y*y + z*z + r))/(x*x + y*y + z*z);
            _z = (z*(x*x + y*y + z*z + r))/(x*x + y*y + z*z);

            cv::Mat adjustedPoint = (cv::Mat_<double>(3,1) << _x,_y,_z);
            return adjustedPoint;
    }

    cv::Mat pointTransform(cv::Mat untransformedPoint){
        cv::Mat untransformedPointmm = (cv::Mat_<double>(3,1) << untransformedPoint.at<double>(0)*1000,untransformedPoint.at<double>(1)*1000,untransformedPoint.at<double>(2)*1000); //Converting m to mm
        cv::Mat transformedPointmm=rotationMatrix*(untransformedPointmm-translationMatrix);
        cv::Mat transformedPoint = (cv::Mat_<double>(3,1) << transformedPointmm.at<double>(0)/1000,transformedPointmm.at<double>(1)/1000,transformedPointmm.at<double>(2)/1000); //Converting m to mm
        return transformedPoint;
    }

};

#endif /* CameraCalibration_h */
