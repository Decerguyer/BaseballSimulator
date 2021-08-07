// V2.h
//
// Baseball Simulator 'define' statements
//
//  Created by Yoni Arieh on 6/18/21.
//

#ifndef V2_h
#define V2_h

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include <cmath>

#define BASEBALL_RADIUS 0.036888
//Baseball radius in meters

#define NUM_CALIBRATION_FRAMES 100
//Number of frames per camera for chessboard calibration

//#define SQUARE_SIZE 152.4 //Big Poster Yoni
//#define SQUARE_SIZE 104.8 //Small Poster Yoni
#define SQUARE_SIZE 145 // fixture board
//Length of singular square on chessboard in mm


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


// void writeMat(cv::Mat mat, std::string fileName){
//     fileName.append(".xml");
//     cv::FileStorage file(fileName, cv::FileStorage::WRITE);

//     file << "saved_matrix" << mat;
//     file.release();
// }

// cv::Mat readMat(std::string fileName){
//     fileName.append(".xml");
//     cv::FileStorage file(fileName, cv::FileStorage::READ);
//     cv::Mat retMatrix;
//     file["saved_matrix"] >> retMatrix;
//     file.release();
//     return retMatrix;
// }


#endif /* V2_h */




