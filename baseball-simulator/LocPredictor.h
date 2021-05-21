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
    
    cv::Rect ROIPrediction(float timeStamp){
        if (secondMeas < 2){
            ROI = imageSize;
        }
        else{
            coord2D predPos = pixelPosPredictor(timeStamp);
            ROI = cv::Rect(predPos.x - ROISize*1.5, predPos.y - ROISize*1.5, ROISize*3, ROISize*3);
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
            return 1;
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
    
    float radiusPred(float depth, const struct rs2_intrinsics * intrin){
        float point[3];
        float pixel[2] = {intrin->ppx, intrin->ppy};
        std::cout << "Intrinsics = " << intrin->ppx << " " << intrin->ppy << std::endl;

        rs2_deproject_pixel_to_point(point, intrin, pixel, depth);
        point[2] += BASEBALL_RADIUS;

        std::cout << "point = " << point[0] << " " << point[1] << " " << point[2] << std::endl; 


        float topEdge[3] = {point[0], point[1] + BASEBALL_RADIUS, point[2]};

        std::cout << "top edge point = " << topEdge[0] << " " << topEdge[1] << " " << topEdge[2] << std::endl; 

        rs2_project_point_to_pixel(pixel, intrin, topEdge);

        std::cout << "top edge pixel = " << pixel[0] << " " << pixel[1] << std::endl; 
        
        return pixel[1]-(intrin->ppy);
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
