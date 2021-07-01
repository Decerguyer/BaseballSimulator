#include "BaseballSimulator.h"
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/aruco.hpp"
#include "ImageReconditioning.h"
#include "V2.h"

ImageReconditioning::ImageReconditioning() {
    Clahe = cv::createCLAHE(); //Creating the clahe object
    sharpnessKernel = (cv::Mat_<float>(3,3) << -1,-1,-1,-1,9,-1,-1,-1,-1); //Small kernel
    //sharpnessKernel = (cv::Mat_<float>(5,5) << -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,25,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1); //Larger Kernel
}

void ImageReconditioning::clahe(cv::Mat src, cv::Mat dst, int clipLimit = 40){ //Constrast Limited Adaptive Histogram Equalization
    Clahe->setClipLimit(clipLimit);
    Clahe->apply(src,dst);
}	

void ImageReconditioning::crop(cv::Mat src, cv::Mat dst, float pX1, float pY1, float pX2, float pY2){ //Frame Crop (note PX2 should be greater than PX1 and PY2 less than PY1)
    if(pX1 < 0){pX1=0;}
    if(pX2 > 847){pX2=847;}
    if(pY1 < 0){pY1=0;}
    if(pY2 > 479){pY2=479;}
    dst = cv::Mat(src,cv::Rect(pX1,pY1,pX2-pX1,pY2-pY1)); //The actual openCV function takes px1, px2, width, height
}

void ImageReconditioning::scale(cv::Mat src, cv::Mat dst, float scaleFactor){ //For image decimation scale factor < 1
    cv::resize(src, dst, cv::Size(), scaleFactor, scaleFactor, cv::INTER_AREA); //INTER_AREA seems to work best. Check documentation for more information
}

void ImageReconditioning::sharpen(cv::Mat src, cv::Mat dst){
    cv::filter2D(src,dst,-1,sharpnessKernel);
}

void ImageReconditioning::guassianBlur(cv::Mat src, cv::Mat dst,int dimmension=9){ //Dimmension = size of kernel (we will use a default of 9x9)
    cv::GaussianBlur(src,dst,cv::Size(dimmension,dimmension),0);
}

void ImageReconditioning::medianBlur(cv::Mat src, cv::Mat dst, int kernelSize){
    cv::medianBlur(src,dst, kernelSize);
}

void ImageReconditioning::otsuThresh(cv::Mat src, cv::Mat dst, int thresh = 0, int maxValue = 255){
    cv::threshold(src,dst,thresh,maxValue,cv::THRESH_OTSU);
}

void ImageReconditioning::equalizeHist(cv::Mat src, cv::Mat dst){
    cv::equalizeHist(src, dst);
}

void ImageReconditioning::canny(cv::Mat src, cv::Mat dst, int lowerThresh, int upperThresh){
    cv::Canny(src, dst, lowerThresh, upperThresh);
}

//positive number increases brightness and negative decreases
void ImageReconditioning::brightness(cv::Mat src, cv::Mat dst, int brightness){
    src.convertTo(dst, -1, 1, brightness);
}

//positive number increases brightness and negative decreases
void ImageReconditioning::contrast(cv::Mat src, cv::Mat dst, int contrastMultiplier){
    src.convertTo(dst, -1, contrastMultiplier, 0);
}

void ImageReconditioning::adaptiveThreshold(cv::Mat src, cv::Mat dst, int blockSize){
    cv::adaptiveThreshold(src, dst, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, blockSize, 0);
}