//Record Frames Below

//
//  FrameRecorder.cpp
//  baseball-test
//
//  Created by Jordan Mayor on 1/1/21.
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


int main(){
    rs2::context ctx; // Create librealsense context for managing devices
    
    auto devList = ctx.query_devices(); // Get a snapshot of currently connected devices
    
    rs2::device dev = devList.front();
    
    auto depthSensor = dev.first<rs2::depth_sensor>();
    
    //float exposureTime = 1300; //unit is microseconds check 300 FPS white paper for more details
    float exposureTime = 1300;
    depthSensor.set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, false);
    depthSensor.set_option(RS2_OPTION_EXPOSURE, exposureTime);
    depthSensor.set_option(RS2_OPTION_GAIN, 16.f);
    depthSensor.set_option(RS2_OPTION_EMITTER_ENABLED, true);
    depthSensor.set_option(RS2_OPTION_EMITTER_ON_OFF, false);
    
    
    
    rs2::config* config = new rs2::config();

    
    int width = 848;
    int height = 480;
    int fps = 90;
    
    // Request a specific configuration
    config->enable_stream(RS2_STREAM_DEPTH, width, height, RS2_FORMAT_Z16, fps);
    config->enable_stream(RS2_STREAM_INFRARED, 1, width, height, RS2_FORMAT_Y8, fps);
    
    // Declare RealSense pipeline, encapsulating the actual device and sensors
    rs2::pipeline pipe;

    rs2::colorizer color_map;
    //rs2::frame_queue q(12);
    
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
        
        for (int i = 0; i < 100; i++){
            rs2::frameset f = pipe.wait_for_frames();
            
            f.keep();
            frames.push_back(f);
        }
        
        std::cout << "Processing..." << std::endl;
        recording = false;
        pipe.stop();
        
        float minThreshDist = 0.3048f;
        float maxThreshDist = 0.3048f * 4.f;
                
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
            applyColorMap(ir_mat, ir_mat, COLORMAP_WINTER);

            Mat filteredDepthMat = denoise(depth_mat);

            float timeStamp = f.get_frame_metadata(RS2_FRAME_METADATA_SENSOR_TIMESTAMP);
                
            timeStampsVec.push_back(timeStamp);
                
            frameNums.push_back(int(f.get_frame_number()));

            images.push_back(ir_mat);
            filteredDepthImages.push_back(filteredDepthMat);
            originalDepthImages.push_back(depth_mat.clone());
            
        }

        for (int i = 0; i < (int)images.size(); (i >= 0 ? i++ : i = 0)){
            std::cout << "TimeStamp = " << timeStampsVec[i] << "; ";
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
        timeStampsVec.clear();
        filteredDepthImages.clear();
        originalDepthImages.clear();
        frameNums.clear();
        images.clear();
        frames.clear();
        
    }
}
