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
    cam.enableStreams(848, 480, 90);
    cam.setExposure(300);
    
    ThresholdFilter threshFilter(cam);
    
    Tracker trk(848, 480, cam.getIntrinsics());

    int a;
    std::cout << "Record Data?\n";
    std::cin >> a;


    std::deque<ImageData> images = cam.recordImageData(100);

    for (int i = 0; i < images.size(); i++){
        images[i].depthMat = threshFilter.filter(images[i].depthMat);
        images[i].depthVisMat = images[i].depthToVisual(images[i].depthMat);
        
        coord2D coordinate = trk.track(images[i]);

        std::cout << i << std::endl;
    }
    
    Visualizer vis;
    
    vis.visualize(images, true, true, true);
}

