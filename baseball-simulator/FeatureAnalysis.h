//
//  FeatureAnalysis.h
//  librealsense2
//
//  Created by Yoni Arieh on 4/15/21.
//

#ifndef FeatureAnalysis_h
#define FeatureAnalysis_h

#include <opencv2/features2d.hpp> //This is used for SIFT
#include "opencv2/aruco.hpp"
#include "V2.h"

class FeatureAnalysis{
public:
    FeatureAnalysis();
    std::vector<cv::KeyPoint> SIFT(cv::Mat src);
    cv::Mat descriptors(cv::Mat src, std::vector<cv::KeyPoint> keypoints);
    std::vector<cv::DMatch> FLANN(cv::Mat src1, cv::Mat src2, cv::Mat descriptors1, cv::Mat descriptors2);
    std::vector <cv::Vec3f> houghCircles(cv::Mat src, double distance, double minRad = 5.0, double maxRad = 50.0);

private:
	cv::Mat dst;
    cv::Ptr<cv::SIFT> siftPtr;
    std::vector<cv::KeyPoint> keypoints;
    cv::Ptr<cv::DescriptorMatcher> matcher;
    
};

#endif /* FeatureAnalysis_h */
