//
//  stagnantMeasurement3D.cpp
//  baseball-test
//
//  Created by Yoni Arieh on 1/12/21.
//

#include <stdio.h>
#include "BaseballSimulator.h"
#include "ImageReconditioning.h"
#include "FeatureAnalysis.h"
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <iostream>             // for cout
#include <vector>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/aruco.hpp"

//Global Variables (TEMPORARY)
int click[2][2] = {{0,0},{0,0}};

static void onMouse( int event, int x, int y, int f, void* ){
    if (event == cv::EVENT_LBUTTONDOWN){
        std::cout << x << " " << y << std::endl;
        click[0][0] = click[1][0];
        click[0][1] = click[1][1];
        click[1][0] = x;
        click[1][1] = y;
    }
}


int main(){
    //Image Proccessor Initialization Block
    ImageReconditioning Reconditioner;
    FeatureAnalysis FeatureAnalyzer;
    //Camera Initialization Block
    Camera cam;    //Create Camera Object
    cam.enableStreams(848, 480, 90);    //Specify stream parameters
    cam.setDefaultSettings();
    cam.disableEmitter();
    cam.setExposure(1000);
         
    //Recording Block
    std::cout << "Enter the integer number of frames to measure: ";    //Give user time to set up the ball
    int num;
    std::cin >> num;
    
    std::deque<ImageData> frames;
    frames = cam.recordImageData(num,30);    //(n,m) Record and save 'n' frames for measurements throw out first 'm' frames
    
    //Keypoint Storage
    std::vector<std::vector<cv::KeyPoint>> keypoints;
    
    //Enhanced Frame storage
    std::vector<cv::Mat> enhancedFrames;
    
    //Image Cropping & enhancement block
    for(int i =0; i<(int)frames.size(); i++){
        //Frame management block
        cv::Mat irMAT = frames[i].getIRMat();
        cv::imshow("Image", irMAT);
        
        //Uncropped Image user feedback
        cv::Mat brightened;
        brightened = Reconditioner.clahe(irMAT);
            
        //Manual Crop Block
        //Blocking Cropper
        std::cout<<"Press any key to continue: ";
        cv::namedWindow( "Brightened", cv::WINDOW_AUTOSIZE);
        cv::imshow("Brightened", brightened);
        cv::setMouseCallback("Brightened", onMouse, 0 );
        cv::waitKey(0);
        cv::Mat cropped;
        cropped = irMAT.clone();
        cropped = Reconditioner.crop(cropped,click[0][0],click[0][1],click[1][0],click[1][1]);
        /*
        //Enchancement Block
        cv::Mat enhanced;
        enhanced = Reconditioner.clahe(cropped);
        */
        //Binarizing Block
        cv::Mat binarized;
        binarized = Reconditioner.otsuThresh(cropped);
        enhancedFrames.push_back(binarized);
        
        //Visualization Block
        cv::imshow("Cropped", cropped);
        //cv::imshow("Enhanced", enhanced);
        cv::imshow("Binarized", binarized);
        
        int key = cv::waitKey(0);
            if (key == 'n'){
                continue;
            }
            else if (key == 27)
                break;
        }
    cv::destroyAllWindows();
    cv::waitKey(1);
    
    for(int i=0; i<(int)enhancedFrames.size(); i++){ //initial i = 1!!!!!!!!
        cv::Mat tmp = enhancedFrames[i].clone();
        //Keypoint finder Block
        keypoints.push_back(FeatureAnalyzer.SIFT(tmp));
        cv::Mat keys;
        cv::drawKeypoints(tmp, keypoints[i], keys);
        
        if(i > 0){
            cv::Mat descriptors1, descriptors2;
            descriptors1 = FeatureAnalyzer.descriptors(enhancedFrames[i-1],keypoints[i-1]);
            descriptors2 = FeatureAnalyzer.descriptors(enhancedFrames[i],keypoints[i]);
            std::vector<cv::DMatch> featureMatches;
            featureMatches = FeatureAnalyzer.FLANN(enhancedFrames[i-1], enhancedFrames[i], descriptors1,descriptors2);
            //-- Draw matches
            cv::Mat img_matches;
            cv::drawMatches(enhancedFrames[i-1], keypoints[i-1], enhancedFrames[i], keypoints[i], featureMatches, img_matches, cv::Scalar::all(-1),cv::Scalar::all(-1), std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
            //-- Show detected matches
            cv::imshow("Good Matches", img_matches );
        }
        cv::imshow("Enhanced Frame", enhancedFrames[i]);
        int key = cv::waitKey(0);
            if (key == 'n'){
                if (i == (int)frames.size() - 1)
                    i--;
                else
                    continue;
            }
            else if (key == 27)
                break;
            else
                i -= 2;
    }
    cv::destroyAllWindows();
    cv::waitKey(1);
    frames.clear();
    keypoints.clear();
    enhancedFrames.clear();
}
