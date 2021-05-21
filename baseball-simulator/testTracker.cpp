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
#include "Camera.h"
#include "ImageData.h"
#include "ThresholdFilter.h"
#include "LocPredictor.h"
#include "Tracker.h"
#include "Visualizer.h"


int main(){

    Camera cam;
    cam.enableStreams(848, 480, 90);
    cam.setExposure(2400);
    
    ThresholdFilter threshFilter(cam);
    
    std::cout << "Intrinsics = " << cam.getIntrinsics().ppx << " " << cam.getIntrinsics().ppy << std::endl;
    Tracker trk(848, 480, cam.getIntrinsics(), threshFilter);

    int a;
    std::cout << "Record Data?\n";
    std::cin >> a;


    std::deque<ImageData> images = cam.recordImageData(100);

    for (int i = 0; i < images.size(); i++){        
        coord2D coordinate = trk.track(images[i]);

        std::cout << i << std::endl;
    }
    
    Visualizer vis;
    
    vis.visualize(images, true, true, true);
}

