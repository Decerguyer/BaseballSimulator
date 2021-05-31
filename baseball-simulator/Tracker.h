#ifndef Tracker_h
#define Tracker_h

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include <cmath>
#include "ThresholdFilter.h"
#include "ImageReconditioning.h"

class Tracker{
public:
    Tracker(int width, int height, struct rs2_intrinsics intrinsics, ThresholdFilter &threshFilter)
    : intrin(intrinsics), threshFilter(threshFilter)
    {
        locPred = LocPredictor(width, height);        
    }
    
    coord2D track(ImageData &imgData){
        float predDepth = locPred.depthPred(imgData.getTimeStamp());
        if (predDepth < 0.8){
            predDepth = 0.8;
        }
        imgData.depthMat = threshFilter.filter(imgData.depthMat, 0.6096 ,(1.2*predDepth));
        imgData.depthVisMat = imgData.depthToVisual(imgData.depthMat);

        coord2D ballCoordDepth = findBallFromDepth(imgData);
        coord2D ballCoordIR = {0, 0 , 0};
        if (ballCoordDepth.depth){
            std::cout << "here Depth = " << ballCoordDepth.depth << std::endl;
            ballCoordIR = findBallFromIR(imgData, imgData.depthVisBallLoc, ballCoordDepth);
        }
        if (ballCoordIR.depth){
            locPred.updateROIPredictor(ballCoordIR, imgData.getTimeStamp(), imgData.IRBallLoc[2]);
            return ballCoordIR;
        }
        else if (ballCoordDepth.depth){
            locPred.updateROIPredictor(ballCoordDepth, imgData.getTimeStamp(), imgData.depthVisBallLoc[2]);
            return ballCoordDepth;
        }
        else{
            return {0, 0, 0};
        }
    }

    coord2D findBallFromDepth(ImageData &imgData){
        imgData.depthVisMatCropped = cropFromPred(imgData.getDepthVisMat(), imgData.getTimeStamp());
        cv::Vec3f ballCircle = depthVisHoughCircle(imgData, 0.15);
        
        cv::Point offset = ROIOffset(imgData.depthVisMatCropped);
        ballCircle[0] = ballCircle[0] + offset.x;
        ballCircle[1] = ballCircle[1] + offset.y;
        imgData.depthVisBallLoc = ballCircle;
        
        float meanDepth = 0;
        if (ballCircle[2]){
            meanDepth = findAveDepth(imgData, ballCircle);
            if (meanDepth < 0.1524)
                meanDepth = 0;
        }            
        coord2D ball2DCoord = {ballCircle[0], ballCircle[1], meanDepth};
        return ball2DCoord;
    }
    
    coord2D findBallFromIR(ImageData &imgData, cv::Vec3f ballCircleDepth, coord2D ballCoordDepth){
        imgData.irMatCropped = cropIR(imgData, ballCircleDepth, 0.5);

        //imgRecondition.medianBlur(imgData.irMatCropped, imgData.irMatCropped, 3);
        //imgRecondition.clahe(imgData.irMatCropped, imgData.irMatCropped, 5);
        //imgRecondition.contrast(imgData.irMatCropped, imgData.irMatCropped, 5);
        //imgRecondition.sharpen(imgData.irMatCropped, imgData.irMatCropped);
        imgRecondition.equalizeHist(imgData.irMatCropped, imgData.irMatCropped);
        //imgRecondition.clahe(imgData.irMatCropped, imgData.irMatCropped, 30);
        //imgRecondition.adaptiveThreshold(imgData.irMatCropped, imgData.irMatCropped, 41);
        //imgRecondition.canny(imgData.irMatCropped, imgData.irMatCropped, 25, 50);




        int radius = locPred.radiusPred(ballCoordDepth, &intrin);

        cv::Vec3f ballCircleIR = irHoughCircle(imgData, radius, 0.02);
        
        cv::Point offset = ROIOffset(imgData.irMatCropped);
        ballCircleIR[0] = ballCircleIR[0] + offset.x;
        ballCircleIR[1] = ballCircleIR[1] + offset.y;
        imgData.IRBallLoc = ballCircleIR;
        
        if (ballCircleIR[2]){
            imgData.irMatCropped = cropIR(imgData, ballCircleIR, 0.05);
            return {ballCircleIR[0], ballCircleIR[1], ballCoordDepth.depth};
        }
        else{
            return {0, 0, 0};
        }
    }

    std::vector<std::vector<float>> convertTo3D(std::vector<coord2D> coord2DVec){
        std::vector<std::vector<float>> positions3DVec;
        for (int i = 0; i < coord2DVec.size(); i++){
            //if (coord2DVec[i].depth){
                float pixel[2] = {coord2DVec[i].x, coord2DVec[i].y};
                float point[3];
                rs2_deproject_pixel_to_point(point, &intrin, pixel, coord2DVec[i].depth);
                std::vector<float> position = {point[0], point[1], point[2]};
                positions3DVec.push_back(position);
            //}
        }
        return positions3DVec;
    }


    

private:
    cv::Mat croppedDepthVis;

    int prevRadius = 0;
    
    struct rs2_intrinsics intrin;
    
    LocPredictor locPred;

    ThresholdFilter &threshFilter;

    ImageReconditioning imgRecondition;

    
    
    //returns pixel x [0], pixel y [1], and the radius[2]
    cv::Vec3f depthVisHoughCircle(ImageData &imgData, float error){
        std::vector<cv::Vec3f> coords;

        //Should eventually refine parameters again
        int minRadius = std::floor(prevRadius-prevRadius*error);
        if (minRadius < 5)
            minRadius = 5;
        int maxRadius = std::ceil(prevRadius+prevRadius*error);
        if (maxRadius > 60 || maxRadius == 0)
            maxRadius = 60;

        cv::HoughCircles(imgData.depthVisMatCropped, coords, cv::HOUGH_GRADIENT, 1.5, 4000, 50, 15, minRadius, maxRadius);
        
        if (!coords.empty()){
            return coords[0];
        }
        else{
            imgData.depthVisMatCropped = imgData.depthVisMat;
            //std::cout << "Resorted to entire depth image because couldn't find from cropped version/n";
            cv::HoughCircles(imgData.depthVisMatCropped, coords, cv::HOUGH_GRADIENT, 1.5, 4000, 50, 15, minRadius, maxRadius);
            if (!coords.empty()){
                return coords[0];
            }
            else{
                return cv::Vec3f(0, 0, 0);
            }
        }
    }
                           
    cv::Vec3f irHoughCircle(ImageData &imgData, float radius, float error){
        std::vector<cv::Vec3f> coords;

        int minRadius = std::floor(radius-radius*error);
        int maxRadius = std::ceil(radius+radius*error);

        std::cout << minRadius << " < Radius < " << maxRadius << " and rad = " << radius << std::endl;
        
        //Should eventually refine parameters again
        double param2 = 6;
        cv::HoughCircles(imgData.irMatCropped, coords, cv::HOUGH_GRADIENT, 1.1, 4000, 100, param2, minRadius, maxRadius);
        
        if (!coords.empty()){
            std::cout << "Radius Through depth hough is " << coords[0][2] << std::endl;
            return coords[0];
        }
        else if (locPred.isSecondMeas()){
            imgData.irMatCropped = cropFromPred(imgData.getIRMat(), imgData.getTimeStamp());
            cv::HoughCircles(imgData.irMatCropped, coords, cv::HOUGH_GRADIENT, 1.1, 4000, 100, param2, minRadius, maxRadius);
            if (!coords.empty()){
                std::cout << "Radius Through Crop Prediction is " << coords[0][2] << std::endl;
                return coords[0];
            }
            else{
                std::cout << "Radius not found " << std::endl;
                return cv::Vec3f(0, 0, 0);
            }
        }
        else{
            return cv::Vec3f(0, 0, 0);
        }
    }
    
    cv::Mat cropFromPred(cv::Mat fullSizeMat, float timeStamp){
        cv::Rect ROI = locPred.ROIPrediction(timeStamp);
        return fullSizeMat(ROI);
    }
    
    cv::Mat cropIR(ImageData &imgData, cv::Vec3f ballCircle, float error){
        return imgData.irMat(locPred.createROI(ballCircle, error));
    }
    
    cv::Point ROIOffset(cv::Mat croppedImg){
        cv::Point offset;
        cv::Size wholesize;
        croppedImg.locateROI(wholesize, offset);
        return offset;
    }
    
    float findAveDepth(ImageData &imgData, cv::Vec3f circleCenter){
        float sum = 0;
        int numMeas = 0;
        for (int i = 0; i < 3; i++){
            for (int k = 0; k < 3; k++){
                if (imgData.getDepthAt(circleCenter[0] - 1 + i, circleCenter[1] - 1 + k))
                    numMeas++;
                sum += imgData.getDepthAt(circleCenter[0] - 1 + i, circleCenter[1] - 1 + k);
            }
        }
        float meanDepth = numMeas ? (sum / numMeas) : 0;
        
        return meanDepth;
    }
};

#endif /* Tracker_h */
