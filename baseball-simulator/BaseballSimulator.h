//
//  BaseballSimulator.h
//
//  Created by Jordan Mayor on 1/8/21.
//  Revamped in V2 by Yoni Arieh on 7/3/21.

#ifndef BaseballSimulator_h
#define BaseballSimulator_h

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include "Camera.h"
#include "D400.h"
#include "DataStructure.h"
//#include "FeatureAnalysis.h"
#include "ImageData.h"
#include "ImageReconditioning.h"
#include "json.hpp"
#include "LocPredictor.h"
#include "sendPost.h"
#include "ThresholdFilter.h"
#include "Tracker.h"
#include "Visualizer.h"
#include "V2.h"

class BaseballSimulator{
    public:
        BaseballSimulator();
        void MainTestingRoutine();
        void savingRoutine(std::string str);
        void loadingRoutine(std::string str);
        void 
        
    private:
        rs2::context ctx;
};


#endif /* BaseballSimulator_h */




