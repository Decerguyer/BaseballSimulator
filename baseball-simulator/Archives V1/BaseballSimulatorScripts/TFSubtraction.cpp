//Thresholdfilter Tracking below


//
//  TFSubraction.cpp
//  baseball-test
//
//  Created by Jordan Mayor on 1/10/21.
//

#include <stdio.h>
#include "BaseballSimulator.h"
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API


int main(){
    Camera cam;
    cam.enableStreams(848, 480, 90);

    ThresholdFilter threshFilter;

    cam.startStream(180);
    while(1){
        rs2::frameset fset = cam.getFrame();


        cv::Mat depthMat = rsDepthToCVMat(fset.get_depth_frame());
        
        depthMat = threshFilter.filter(depthMat);


//        cv::Mat depthMatVisualized;
//        depthMat.convertTo(depthMatVisualized, CV_8UC1);
//
//        cv::equalizeHist( depthMatVisualized, depthMatVisualized);
//        cv::applyColorMap(depthMatVisualized, depthMatVisualized, cv::COLORMAP_JET);

        cv::imshow("Depth Thresholded", depthMat);

        cv::waitKey(1);
    }
    cam.endStream();
}



//rs2::pipeline pipe;
//
//pipe.start();
//
//for (int i = 0; i < 5; i ++){
//auto const intrin = pipe.get_active_profile().get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>().get_intrinsics();
//
//
//float tdPoint[3];
//float pixel[2] = {2, 2};
//
//float distance = 1;
//
//rs2_deproject_pixel_to_point(tdPoint, &intrin, pixel, distance);



