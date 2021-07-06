//Stationary Ball Measuring Tool below


//
//  stagnantMeasurement3D.cpp
//  baseball-test
//
//  Created by Yoni Arieh on 1/12/21.
//

#include <stdio.h>
#include "BaseballSimulator.h"
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <iostream>             // for cout
#include <vector>
#include "CameraCalibration.h"

int main(){
    CameraCalibration calibrationParameters;
    cv::Mat rotationMatrix,translationMatrix;
    rotationMatrix = calibrationParameters.getRotationMatrix();
    translationMatrix = calibrationParameters.getTranslationMatrix();
    std::cout << "Rotation Matrix: " << rotationMatrix << std::endl;
    std::cout << "Translation Matrix: " << translationMatrix << std::endl;
    return 0;
}
