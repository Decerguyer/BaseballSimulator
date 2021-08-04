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
    coord2D track(ImageData imgData);
    static void onMouse( int event, int x, int y, int flag, void* );
    void onMouse(int event, int x, int y, int flag);

    ImageData originalIRMat;

    private:
    struct circleDimensions{
        bool paused = true;

        int initialX;
        int initialY;
        int currentX;
        int currentY;
    } circleDims;


};

#endif /* PseudoTracker_h */