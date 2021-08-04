#include "PseudoTracker.h"

coord2D PseudoTracker::track(ImageData imgData){
    originalIRMat = imgData.irMat.clone();

    cv::Vec3f circ;
    while(true){
        Visualizer::visualizeSingle(imgData, true, true, false);
        cv::setMouseCallback("IR", onMouse, this);

        circ[0] = (circleDims.currentX + circleDims.initialX)/2;
        circ[1] = (circleDims.currentY - circleDims.initialY)/2;
        circ[2] = std::sqrt(std::pow(circ[0]-circleDims.initialX, 2) + std::pow(circ[1]-circleDims.initialY, 2));
        Visualizer::drawCircle(imgData.irMat, circ);

        imgData.irMat = originalIRMat.clone();
    }
    imgData.irMat = originalIRMat;

}

static void PseudoTracker::onMouse( int event, int x, int y, int flag, void* userData){
    PseudoTracker* pseudoTrkPtr = (PseudoTracker*)userData;
    pseudoTrkPtr->onMouse(event, x, y);
}

void PseudoTracker::onMouse(int event, int x, int y, int flag){
    if (event == cv::EVENT_LBUTTONDOWN) {
        circleDims.initialX = x;
        circleDims.initialY = y;
        circleDims.paused = false;
    }
    else if (event == cv::EVENT_LBUTTONUP) {
        circleDims.currentX = x;
        circleDims.currentY = y;
        circleDims.paused = true
    }
    //Mouse move coordinates update
    else if (event == cv::EVENT_MOUSEMOVE) {
        if (!circleDims.paused){
            circleDims.currentX = x;
            circleDims.currentY = y;
        }
    }

    if (flag == cv::CV_EVENT_FLAG_SHIFTKEY){
        circleDims.success = true;
    }
}
