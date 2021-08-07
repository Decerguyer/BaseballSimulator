#include "PseudoTracker.h"

PseudoTracker::PseudoTracker(){
}

coord2D PseudoTracker::track(ImageData imgData){
    originalIRMat = imgData.irMat.clone();
    circleDims.reset();
    std::cout << "TimeStamp = " << imgData.getTimeStamp() << "; ";
    std::cout << "Act Frame # = " << imgData.getFrameNumber() << std::endl;

    cv::namedWindow( "IR", cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback("IR", onMouse, this);

    cv::Vec3f circ = {0, 0, 0};
    while(true){
        Visualizer::drawCircle(imgData.irMat, circ);
        Visualizer::visualizeSingleSetUp(imgData, true, true, false);
        int key = cv::waitkey(10);
        if (key == 'n'){
            break;
        }

        circ[0] = (circleDims.currentX + circleDims.initialX)/2;
        circ[1] = (circleDims.currentY + circleDims.initialY)/2;
        circ[2] = std::sqrt(std::pow(circ[0]-circleDims.initialX, 2) + std::pow(circ[1]-circleDims.initialY, 2));

        imgData.irMat = originalIRMat.clone();
    }
    cv::destroyAllWindows();
    cv::waitKey(1);

    imgData.irMat = originalIRMat;

    coord2D coord = {circ[0], circ[1], imgData.getDepthAt(circ[0], circ[1])};
    std::cout << "Coord2D = " << coord;
    return coord;
}

void PseudoTracker::onMouse( int event, int x, int y, int flag, void* ptr){
    PseudoTracker* pseudoTrkPtr = (PseudoTracker*)ptr;
    pseudoTrkPtr->onMouse(event, x, y, flag);
}

void PseudoTracker::onMouse(int event, int x, int y, int flag){
    if (event == cv::EVENT_LBUTTONDOWN) {
        circleDims.initialX = x;
        circleDims.initialY = y;
        circleDims.currentX = x;
        circleDims.currentY = y;
        circleDims.paused = false;
    }
    else if (event == cv::EVENT_LBUTTONUP) {
        circleDims.currentX = x;
        circleDims.currentY = y;
        circleDims.paused = true;
    }
    //Mouse move coordinates update
    else if (event == cv::EVENT_MOUSEMOVE) {
        if (!circleDims.paused){
            circleDims.currentX = x;
            circleDims.currentY = y;
        }
    }
    else if (event == cv::EVENT_RBUTTONDOWN){
        circleDims.reset();
    }
    // if (flag == cv::EVENT_FLAG_SHIFTKEY){
    //     circleDims.success = true;
    // }
}

void PseudoTracker::circleDimensions::reset(){
    paused = true;
    success = false;
    initialX = 0;
    initialY = 0;
    currentX = 0;
    currentY = 0;
}

