//Stationary Ball Measuring Tool below


//
//  testTracker.cpp
//  baseball-test
//
//

#include <stdio.h>
#include "BaseballSimulator.h"
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <iostream>             // for cout
#include <vector>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/aruco.hpp"


int main(){

    Camera cam;
    cam.enableIRStream(424, 240, 6);
    
    ThresholdFilter threshFilter(cam);
    
    std::deque<ImageData> images = cam.recordImageData(20);
    
    Tracker trk(848, 480, cam.getIntrinsics());
    
    for (int i = 0; i < images.size(); i++){
        //images[i].depthMat = threshFilter.filter(images[i].depthMat);
        
        //coord2D ballCoordDepth = trk.findBallFromDepth(images[i]);
        //coord2D ballCoordIR = trk.findBallFromIR(images[i], images[i].depthVisBallLoc, ballCoordDepth.depth);
    }
    
    Visualizer vis;
    
    vis.visualize(images, true, true, true);
}

