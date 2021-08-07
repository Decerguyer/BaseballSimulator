#ifndef PseudoTracker_h
#define PseudoTracker_h

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>
#include <deque>
#include <cmath>
#include "Visualizer.h"
#include "V2.h"

class PseudoTracker{
    public:
    PseudoTracker();
    coord2D track(ImageData imgData);
    static void onMouse( int event, int x, int y, int flag, void* ptr);
    void onMouse(int event, int x, int y, int flag);

    private:
    struct circleDimensions{
        bool paused = true;
        bool success;

        int initialX;
        int initialY;
        int currentX;
        int currentY;

        void reset();
    } circleDims;
    
    cv::Mat originalIRMat;

};

#endif /* PseudoTracker_h */
