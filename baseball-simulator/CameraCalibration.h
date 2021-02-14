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


class CameraCalibration{
public:
    CameraCalibration() {
        createCalibration();
    }
    
    cv::Mat getRotationMatrix(){
        return rotationMatrix;
    }
    cv::Mat getTranslationMatrix(){
        return translationMatrix;
    }
    
    
private:
    Camera camera;
    cv::Mat rotationMatrix = (cv::Mat_<double>(3,3) << 0,0,0,0,0,0,0,0,0);
    cv::Mat translationMatrix = (cv::Mat_<double>(3,1) << 0,0,0);
    const int numCalibrationFrames = 100;
    //const int CHECKERBOARD[2]={4,4}; //Medium Chckerboard
    //const float squareSize = 28.575; //in mm
    //const int CHECKERBOARD[2]={3,3}; //Dimensions of CheckerPattern in corners
    //const float squareSize = 39.6875; //Square length in mm   
    int rows = 4;
    int columns = 5;
    int CHECKERBOARD[2]={columns,rows}; //Printout Chckerboard
    float squareSize = 76.2; //in mm
    
    void createCalibration(){
        camera.setDefaultSettings();
        camera.disableEmitter();
        std::vector<rs2::frameset> frames;
        frames = camera.recordRSFrames(numCalibrationFrames,30);	//(n,m) Record and save 'n' frames for measurements throw out first 'm' frames
        
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
        
        for(int i =0; i<(int)frames.size();(i >= 0 ? i++ : i = 0)){	//Iterates through the vector of framesets
        	rs2::frameset fset = frames[i];	//Pulls the current frameset
        	cv::Mat irMAT = cv::Mat(cv::Size(848, 480), CV_8UC1, (void*)fset.get_infrared_frame(1).get_data());
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
    			int key = cv::waitKey(0);
    			cv::destroyAllWindows();
    			cv::waitKey(1);
    			frames.clear();
    			//return 0;
    		}
    		corner_pts = sortCornerPoints(corner_pts,rows,columns);
    		auto intrin = camera.getIntrin();
    		cv::Mat cameraMatrix = (cv::Mat_<float>(3,3) << intrin.fx,0,intrin.ppx,0,intrin.fy,intrin.ppy,0,0,1);
    		cv::Mat distCoeffs = (cv::Mat_<float>(1,5) << 0,0,0,0,0); //Distortion coeff, should be 0 for all usable realsense cameras
    		cv::Mat R,T; //Output Rotation and Translation Vectors
    		cv::solvePnP(objp,corner_pts,cameraMatrix,distCoeffs,R,T,0,0); //Last 0 represents CV_ITERATIVE method
    		
    		cv::Mat rotation, rotationInverse;
    		cv::Rodrigues(R,rotation);
    		cv::invert(rotation,rotationInverse);
		rotationMatrix += rotationInverse;
		translationMatrix += T;
	}
    	rotationMatrix /= numCalibrationFrames;
    	translationMatrix /= numCalibrationFrames; 	
    }
    
    std::vector<cv::Point2f> sortCornerPoints(std::vector<cv::Point2f> cornerPoints,int rows,int columns) {
	std::vector<cv::Point2f> finalSortedCorners;
	for(int j = 0; j < rows; j++){
		std::vector<cv::Point2f> ySortedPoints;
    		for(int i = 0; i < columns; i++) {
    			int smallestYVal = 480;
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
    			int smallestXVal = 848;
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

};

#endif /* CameraCalibration_h */
