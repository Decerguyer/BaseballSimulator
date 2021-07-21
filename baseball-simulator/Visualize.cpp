#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include <cmath>
#include "Visualizer.h"
#include "ImageData.h"
#include "V2.h"

Visualizer::Visualizer(){
}

Visualizer::Visualizer(std::vector<coord2D> *empty_clicks){
    clicks = empty_clicks;
}

//returns a Mat with a circle drawn on it
cv::Mat Visualizer::drawCircle(cv::Mat image, cv::Vec3f coord){
    cv::Mat retMat = image;
    cv::Point pnt(coord[0], coord[1]);

    //draw the outer circle //green
    circle(retMat, pnt, coord[2], cv::Scalar(0,255,0), 2, 8, 0 );
    //draw the center of the circle //red
    circle(retMat, pnt, 2 , cv::Scalar(0,0,255), 2, 8, 0 );

    return retMat;
}

void Visualizer::visualize(std::deque<ImageData> imgData, bool showDepth, bool showIR, bool drawCircles){
    for (int i = 0; i < (int)imgData.size(); (i >= 0 ? i++ : i = 0)){
        std::cout << "TimeStamp = " << imgData[i].getTimeStamp() << "; ";
        //std::cout << "Depth = " << ballDepthVec[i] << "; ";
        std::cout << "Frame = " << i << "/" << imgData.size() - 1 << "; ";
        std::cout << "Act Frame # = " << imgData[i].getFrameNumber() << std::endl;
        
        tmpImgData = &imgData[i];

        if (drawCircles){
            if (imgData[i].depthVisBallLoc[2])
                drawCircle(imgData[i].depthVisMat, imgData[i].depthVisBallLoc);
            if (imgData[i].IRBallLoc[2])
                drawCircle(imgData[i].irMat, imgData[i].IRBallLoc);
        }
        
        if (showDepth){
            imshow("Depth Visualized", imgData[i].depthVisMat);
            imshow("Depth Visualized ROI", imgData[i].depthVisMatCropped);
        }
        if (showIR){
            cv::namedWindow( "IR", cv::WINDOW_AUTOSIZE);
            imshow("IR", imgData[i].irMat);
            cv::setMouseCallback("IR", onMouse, 0 );
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

ImageData* Visualizer::tmpImgData{NULL};
std::vector<coord2D>* Visualizer::clicks{NULL};

void Visualizer::onMouse( int event, int x, int y, int f, void*){
    if (event == cv::EVENT_RBUTTONDOWN){
        std::cout << x << " " << y << std::endl;
        if(clicks != NULL){
            coord2D tmp;
            tmp.x = x;
            tmp.y = y;
            tmp.depth = tmpImgData->getDepthAt(x,y);
            clicks->push_back(tmp);
        }
    }
}