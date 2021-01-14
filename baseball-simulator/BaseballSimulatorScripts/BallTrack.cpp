//TrackAndZoom Program

//
//  trackNzoom.cpp
//  baseball-test
//
//  Created by Jordan Mayor on 12/20/20.
//

#include <stdio.h>
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <iostream>             // for cout
#include <vector>
#include <opencv2/opencv.hpp>
//#include "cv-helpers.hpp"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <string>

using namespace cv;

Mat denoise(Mat depth_mat){
    Mat filtered;
//    bitwise_and(depth_mat, depth_mat, filtered, mask = mask);
    
    
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
//    blur(depth_mat, depth_mat, Size(3, 3));
    
    cvtColor(depth_mat, filtered, COLOR_BGR2GRAY);
    threshold(filtered,filtered, 0, 255, THRESH_BINARY);
    
    dilate(filtered, filtered, kernel);
    erode(filtered, filtered, kernel);


    Canny(filtered, filtered, 50, 100);

    
    return filtered;
}

std::vector <Vec3f> ball_detector(Mat image, double distance){
    std::vector<Vec3f> circles;

    int radius = (27*0.646)/distance;
    
    if (radius > 25){
        radius = 25;
    }
    
    HoughCircles(image, circles, HOUGH_GRADIENT, 1.6, 4000, 50, 5, (radius - radius*0.15), radius + radius*0.15);
    
    return circles;
}



int main(){
    
    rs2::config* config = new rs2::config();

    
    int width = 848;
    int height = 480;
    int fps = 90;
    
    // Request a specific configuration
    config->enable_stream(RS2_STREAM_DEPTH, width, height, RS2_FORMAT_Z16, fps);
    config->enable_stream(RS2_STREAM_INFRARED, 1, width, height, RS2_FORMAT_Y8, fps);
    //config->enable_stream(RS2_STREAM_INFRARED, 2, width, height, RS2_FORMAT_Y8, fps);
    
    // Declare RealSense pipeline, encapsulating the actual device and sensors
    rs2::pipeline pipe;

    rs2::colorizer color_map;
    //rs2::frame_queue q(12);
    
    
    std::vector<float> ballDepthVec;
    std::vector<float> timeStampsVec;
    std::vector<Mat> images;
    std::vector<Mat> filteredDepthImages;
    std::vector<Mat> originalDepthImages;
    std::vector<int> frameNums;
    std::vector<rs2::frameset> frames;
    
    char button[1];
    bool recording = false;
    while(true){
        if (!recording){
            std::cout << "Press a key to start recording" << std::endl;
            std::cin >> button;

            recording = true;
            auto prof = pipe.start(*config);
            std::cout << "Recording..." << std::endl;
        }
        
        //discard first 'n' frames waiting for the stream to stabilize
        for (int i = 0; i < 10; i++){
            rs2::frameset f = pipe.wait_for_frames();
        }
        
        for (int i = 0; i < 180; i++){
            rs2::frameset f = pipe.wait_for_frames();
            
            f.keep();
            frames.push_back(f);
        }
        
        std::cout << "Processing..." << std::endl;
        recording = false;
        pipe.stop();
        
        
        Rect ROI(0, 0, width, height);
        Rect imageSize = Rect(0, 0, width, height);
        
        float minThreshDist = 0.3048f;
        float maxThreshDist = 0.3048f * 4.f;
        
        float pixel[2] = {90, 90};
        
        for (int c = 0; c < (int)frames.size(); c++){
            rs2::frameset fSet = frames[c];

            rs2::frame f = fSet.get_depth_frame();
            rs2::depth_frame df = fSet.get_depth_frame();
            
            rs2::threshold_filter thr_filter(minThreshDist, maxThreshDist);
            f = thr_filter.process(f).apply_filter(color_map);

            Mat depth_mat(Size(width, height), CV_8UC3, (void*)f.get_data(), Mat::AUTO_STEP);

            rs2::video_frame irFrameLeft = fSet.get_infrared_frame(1);
            Mat ir_mat = cv::Mat(Size(width, height), CV_8UC1, (void*)irFrameLeft.get_data());

            equalizeHist( ir_mat, ir_mat);
            applyColorMap(ir_mat, ir_mat, COLORMAP_BONE);

            Mat filteredDepthMat = denoise(depth_mat);
            
            Rect inBoundsROI = (ROI & imageSize);
            
            Mat filteredDepthMatROI = filteredDepthMat(inBoundsROI);
            
            std::vector <Vec3f> circles = ball_detector(filteredDepthMatROI, minThreshDist);

            if (circles.size() >= 1){
                pixel[0] = (circles[0][0]) - (pixel[0] >= 90 ? 90 : pixel[0]) + pixel[0];
                pixel[1] = (circles[0][1]) - (pixel[1] >= 90 ? 90 : pixel[1]) + pixel[1];
                Point pnt(pixel[0], pixel[1]);

                float distance = df.get_distance(pixel[0], pixel[1]);
                ballDepthVec.push_back(distance);
                
                if (distance == 0){
                    distance = minThreshDist;
                }
                //minThreshDist = distance - (0.5f * 0.3048f);
                //maxThreshDist = distance + (0.5f * 0.3048f);

                ROI = Rect(pixel[0] - 90, pixel[1] - 90, 180, 180);
                

                float timeStamp = f.get_frame_metadata(RS2_FRAME_METADATA_SENSOR_TIMESTAMP);
                timeStampsVec.push_back(timeStamp);
                
                frameNums.push_back(int(f.get_frame_number()));


                //draw the outer circle
                circle(ir_mat, pnt, int(circles[0][2]), Scalar(0,255,0), 2, 8, 0 );
                circle(ir_mat, pnt, 2 ,Scalar(0,0,255), 2, 8, 0 );
                //draw the center of the circle
                images.push_back(ir_mat);
                filteredDepthImages.push_back(filteredDepthMat);
                originalDepthImages.push_back(depth_mat.clone());
            }
        }

        for (int i = 0; i < (int)images.size(); (i >= 0 ? i++ : i = 0)){
            std::cout << "TimeStamp = " << timeStampsVec[i] << "; ";
            std::cout << "Depth = " << ballDepthVec[i] << "; ";
            std::cout << "Frame = " << i + 1 << "/" << images.size() << "; ";
            std::cout << "Act Frame # = " << frameNums[i] << std::endl;
            
            
            imshow("Recording_IR", images[i]);
            imshow("Recording_Filtered_Depth", filteredDepthImages[i]);
            imshow("Original Depth", originalDepthImages[i]);
            int key = waitKey(0);
            if (key == 'n'){
                if (i == (int)images.size() - 1)
                    i--;
                else
                    continue;
            }
            else if (key == 27)
                break;
            else
                i -= 2;
        }
        destroyAllWindows();
        waitKey(1);
        ballDepthVec.clear();
        timeStampsVec.clear();
        filteredDepthImages.clear();
        originalDepthImages.clear();
        frameNums.clear();
        images.clear();
        frames.clear();
    }
}
