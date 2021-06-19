// V2.h
//
// Baseball Simulator 'define' statements
//
//  Created by Yoni Arieh on 6/18/21.
//

#ifndef V2_h
#define V2_h

#include <deque>
#include <cmath>

#define BASEBALL_RADIUS 0.036888
//Baseball radius in meters

#define numCalibrationFrames 100
//Number of frames per camera for chessboard calibration

#define squareSize 152.4
//Length of singular square on chessboard in mm


struct coord2D{
    float x;
    float y;
    float depth;
};

struct coord3D{
    float x;
    float y;
    float z;
};

#endif /* V2_h */




