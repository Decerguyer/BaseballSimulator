//
//  FeatureAnalysis.h
//  librealsense2
//
//  Created by Yoni Arieh on 4/15/21.
//

#ifndef FeatureAnalysis_h
#define FeatureAnalysis_h

#include <opencv2/features2d.hpp> //This is used for SIFT


class FeatureAnalysis{
public:
    FeatureAnalysis() {
        siftPtr = cv::SIFT::create(); //Creating the sift object
        matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);
    }
    std::vector<cv::KeyPoint> SIFT(cv::Mat src){
        siftPtr->detect(src, keypoints);
        // Add results to image and save.
        //cv::drawKeypoints(src, keypoints, dst);
        return keypoints;
    }
    cv::Mat descriptors(cv::Mat src, std::vector<cv::KeyPoint> keypoints){
        cv::Mat descriptors;
        siftPtr->compute(src,keypoints,descriptors);
        return descriptors;
    }
    
    std::vector<cv::DMatch> FLANN(cv::Mat src1, cv::Mat src2, cv::Mat descriptors1, cv::Mat descriptors2){
        std::vector< std::vector<cv::DMatch>> knn_matches;
        matcher->knnMatch( descriptors1, descriptors2, knn_matches, 2 );
        //-- Filter matches using the Lowe's ratio test
        const float ratio_thresh = 0.7f;
        std::vector<cv::DMatch> good_matches;
        for (size_t i = 0; i < knn_matches.size(); i++)
        {
            if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance)
            {
                good_matches.push_back(knn_matches[i][0]);
            }
        }
        
        return good_matches;
    }
    
    std::vector <cv::Vec3f> houghCircles(cv::Mat src, double distance, double minRad = 5.0, double maxRad = 50.0){ //Hough Circle Transform
        std::vector<cv::Vec3f> circles;

        int radius = (24*0.646)/distance; //This needs to be fixed
        
        //cv::HoughCircles(image, circles, cv::HOUGH_GRADIENT, 1.6, 4000, 50, 5, (radius - radius*0.15), radius + radius*0.15);
        cv::HoughCircles(src, circles, cv::HOUGH_GRADIENT, 1.6, 4000, 50, 5, minRad, maxRad);
        return circles;
    }

    
private:
	cv::Mat dst;
    cv::Ptr<cv::SIFT> siftPtr;
    std::vector<cv::KeyPoint> keypoints;
    cv::Ptr<cv::DescriptorMatcher> matcher;
};

#endif /* FeatureAnalysis_h */
