//Stationary Ball Measuring Tool below


//
//  stagnantMeasurement3D.cpp
//  baseball-test
//
//  Created by Yoni Arieh on 1/12/21.
//

#include <stdio.h>
#include "BaseballSimulator.h"
#include "ImageReconditioning.h"
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
    ImageReconditioning Processor;
    
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
    
    for(int i =0; i<(int)frames.size();(i >= 0 ? i++ : i = 0)){    //Iterates through the vector of framesets
    
    //Frame management block
    cv::Mat irMAT = frames[i].getIRMat();
    cv::imshow("Image", irMAT);   
    
    //Uncropped Image user feedback
    cv::Mat brightened;
    brightened = Processor.clahe(irMAT);
    
    //Manual Crop Block
    cv::namedWindow( "Brightened", cv::WINDOW_AUTOSIZE);
    cv::imshow("Brightened", brightened);
    cv::setMouseCallback("Brightened", onMouse, 0 );
    //std::cout << "Press any key to continue" << std::endl;
    //int block = cv::waitKey(0);
   
    int key = cv::waitKey(0);
        if (key == 'n'){
            if (i == (int)frames.size() - 1)
                i--;
            else
                continue;
        }
        else if (key == 27)
            break;
        else if (key == 'f'){
        	//std::cout << "Enter the 2 corners in order" << std::endl;
    		//cv::Point2f a,b; 
    		//std::cin >> a.x >> a.y >> b.x >> b.y;
    		cv::Mat cropped;
    		cropped = Processor.crop(irMAT,click[0][0],click[0][1],click[1][0],click[1][1]);
   
    		//Sharpening Block
    		cv::Mat sharpened;
    		sharpened = Processor.sharpen(cropped);
    
    		//Enchancement Block
    		cv::Mat enhanced;
    		enhanced = Processor.clahe(sharpened);
   		
    		//Scaling Block
    		cv::Mat scaled;
    		scaled = Processor.scale(enhanced,5);
    		
    		//Guassian Blur Block
    		cv::Mat blur;
    		blur = Processor.guassianBlur(scaled);	
    		
    		//Binarization
   		cv::Mat binary;
   		binary = Processor.otsuThresh(blur);
   		
   		//Extra Sharpening Block
   		cv::Mat tmp = binary.clone();
    		cv::Mat extraSharpened;
    		extraSharpened = Processor.sharpen(tmp);	
    		
		/*    
    		//Decimation Block
    		cv::Mat dec = enhanced.clone();
    		cv::Mat decimated;
    		decimated = Processor.scale(dec,0.5);
    		*/
    		//Visualization Block
    		cv::imshow("Cropped", cropped);
    		cv::imshow("Scaled", scaled);
    		cv::imshow("Sharpened", sharpened);
    		cv::imshow("Enhanced", enhanced);
    		cv::imshow("Binarized", binary);
    		cv::imshow("Extra Sharpen", extraSharpened);
    		//cv::imshow("decimated", enhanced); 
        }
        	
        else
            i -= 2;
    }
    cv::destroyAllWindows();
    cv::waitKey(1);
    frames.clear();      
}
