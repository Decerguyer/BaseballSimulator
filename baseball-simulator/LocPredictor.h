#ifndef LocPredictor_h
#define LocPredictor_h

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include <cmath>


class LocPredictor{
public:
    LocPredictor(){
        ROI = cv::Rect(0, 0, 0, 0);
        imageSize = cv::Rect(0, 0, 0, 0);
        ROISize = 0;
    }
    
    LocPredictor(int width, int height){
        ROI = cv::Rect(0, 0, width - 1, height - 1);
        imageSize = cv::Rect(0, 0, width - 1, height - 1);
        ROISize = 0;
    }

    bool isSecondMeas(){
        return secondMeas >= 2;
    }
    
    cv::Rect ROIPrediction(float timeStamp){
        if (secondMeas < 2){
            ROI = imageSize;
        }
        else{
            coord2D predPos = pixelPosPredictor(timeStamp);
            ROI = cv::Rect(predPos.x - ROISize*2, predPos.y - ROISize*2, ROISize*4, ROISize*4);
        }
        
        cv::Rect inBoundsROI = ROI & imageSize;
        if (inBoundsROI.area()){
            return inBoundsROI;
        }
        else {
            return imageSize;
        }
    }
    
    coord2D pixelPosPredictor(float timeStamp){
        coord2D predPos;
        predPos.x = prevPos.x + velo.x*(timeStamp-prevTimeStamp);
        predPos.y = prevPos.y + velo.y*(timeStamp-prevTimeStamp);
        predPos.depth = prevPos.depth + velo.depth*(timeStamp-prevTimeStamp);
        
        return predPos;
    }

    float depthPred(float timeStamp){
        if (secondMeas == 0){
            return 0.8;
        }
        else if (secondMeas == 1){
            return 0.8 + prevPos.depth;
        }
        else{
            return prevPos.depth + velo.depth*(timeStamp-prevTimeStamp);
        }        
    }
    
    void updateROIPredictor(coord2D currPos, float timeStamp, int currRadius){
        velo.x = (currPos.x - prevPos.x)/(timeStamp-prevTimeStamp);
        velo.y = (currPos.y - prevPos.y)/(timeStamp-prevTimeStamp);
        velo.depth = (currPos.depth - prevPos.depth)/(timeStamp-prevTimeStamp);

        prevPos = currPos;
        prevTimeStamp = timeStamp;
        ROISize = currRadius;
        secondMeas++;
    }
    
    cv::Rect createROI(cv::Vec3f ballCircle, float error){
        float halfSize = ballCircle[2]*(error+1);
        cv::Rect tempROI(ballCircle[0]-halfSize, ballCircle[1]-halfSize, halfSize*2, halfSize*2);
        return tempROI & imageSize;
    }
    
    float radiusPred(coord2D ballLocDepth, const struct rs2_intrinsics * intrin){
        float point[3];
        float origPixel[2] = {ballLocDepth.x, ballLocDepth.y};

        rs2_deproject_pixel_to_point(point, intrin, origPixel, ballLocDepth.depth);
        point[2] += BASEBALL_RADIUS;


        float topEdge[3] =  {point[0], point[1] + (float)BASEBALL_RADIUS, point[2]};
        float rightEdge[3] = {point[0] + (float)BASEBALL_RADIUS, point[1], point[2]};
        float bottomEdge[3] = {point[0], point[1] - (float)BASEBALL_RADIUS, point[2]};
        float leftEdge[3] = {point[0] - (float)BASEBALL_RADIUS, point[1], point[2]};

        float pixelTop[2];
        float pixelRight[2];
        float pixelBottom[2];
        float pixelLeft[2];

        rs2_project_point_to_pixel(pixelTop, intrin, topEdge);
        rs2_project_point_to_pixel(pixelRight, intrin, rightEdge);
        rs2_project_point_to_pixel(pixelBottom, intrin, bottomEdge);
        rs2_project_point_to_pixel(pixelLeft, intrin, leftEdge);

        return ((pixelTop[1]-pixelBottom[1])/2.f + (pixelRight[0]-pixelLeft[0])/2.f)/2.f;
    }

    // void findEdge(float pixel[2], float changeX, float changeY, float point[3]){
    //     point[0] += changeX;
    //     point[1] += changeY;
    //     rs2::rs2_project_point_to_pixel(pixel, intrin, point);
    // }
    
private:
    coord2D prevPos;
    coord2D velo;
    float prevTimeStamp = 0;
    int secondMeas = 0;
    
    int ROISize = 0;
    cv::Rect ROI;
    cv::Rect imageSize;
};

#endif /* LocPredictor_h */
