//
//  ImageReconditioning.h
//  librealsense2
//
//  Created by Yoni Arieh on 4/15/21.
//

#ifndef ImageReconditioning_h
#define ImageReconditioning_h

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/aruco.hpp"
#include "V2.h"


class ImageReconditioning{
public:
    ImageReconditioning();
    void clahe(cv::Mat src, cv::Mat dst, int clipLimit = 40);	
    void crop(cv::Mat src, cv::Mat dst, float pX1, float pY1, float pX2, float pY2);
    void scale(cv::Mat src, cv::Mat dst, float scaleFactor);
    void sharpen(cv::Mat src, cv::Mat dst);
    void guassianBlur(cv::Mat src, cv::Mat dst,int dimmension=9);
	void medianBlur(cv::Mat src, cv::Mat dst, int kernelSize);
    void otsuThresh(cv::Mat src, cv::Mat dst, int thresh = 0, int maxValue = 255);
	void equalizeHist(cv::Mat src, cv::Mat dst);
	void canny(cv::Mat src, cv::Mat dst, int lowerThresh, int upperThresh);
	void brightness(cv::Mat src, cv::Mat dst, int brightness);
	void contrast(cv::Mat src, cv::Mat dst, int contrastMultiplier);
	void adaptiveThreshold(cv::Mat src, cv::Mat dst, int blockSize);
    
private:
	cv::Ptr<cv::CLAHE> Clahe;
	cv::Mat sharpnessKernel;
	
};

#endif /* ImageReconditioning_h */
