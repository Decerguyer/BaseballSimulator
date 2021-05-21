#ifndef Visualizer_h
#define Visualizer_h

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include <cmath>

class Visualizer{

public:
    //returns a Mat with a circle drawn on it
    cv::Mat drawCircle(cv::Mat image, cv::Vec3f coord){
        cv::Mat retMat = image;
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
            std::cout << "Frame = " << i << "/" << imgData.size() - 1 << "; ";
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

#endif /* Visualizer_h */
