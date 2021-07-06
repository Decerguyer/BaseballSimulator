//Tool to help calculate relationship between range and radius


//
//PointToPixelPrj.cpp
//  baseball-test
//
//  Created by Yoni Arieh on 1/13/21.
//

#include <stdio.h>
#include "BaseballSimulator.h"
#include <librealsense2/rsutil.h> // Include RealSense Cross Platform API
#include <iostream>             // for cout
#include <vector>
#include <math.h>

void baseballHorizEdgeCalculator(float Center[3], float edge1[3], float edge2[3]){

	float range = sqrt(Center[0]*Center[0] + Center[2]*Center[2]);
	float cosTheta = Center[0]/range;
	float sinTheta = Center[2]/range;
	float radius = 0.0375; //Radius of baseball in meters
	
	if(Center[0]>=0){ //If equal to, both statements apply
		
		edge1[0] = Center[0]-radius*sinTheta; //Closer Edge (x closer to zero) (z is larger) (relative to center point)
		edge1[1] = Center[1];
    		edge1[2] = Center[2]+radius*cosTheta;
    
    		edge2[0] = Center[0]+radius*sinTheta; //Farther Edge (x is farther from zero) (z is smaller) (relative to center point)
    		edge2[1] = Center[1];
    		edge2[2] = Center[2]-radius*cosTheta;
	
	}
	else{
		edge1[0] = Center[0]+radius*sinTheta; //Closer Edge (x closer to zero) (z is larger) (relative to center point)
		edge1[1] = Center[1];
    		edge1[2] = Center[2]-radius*cosTheta;
    
    		edge2[0] = Center[0]-radius*sinTheta; //Farther Edge (x is farther from zero) (z is smaller) (relative to center point)
    		edge2[1] = Center[1];
    		edge2[2] = Center[2]+radius*cosTheta;
	}
	std::cout << edge1[0] << std::endl;
	std::cout << edge1[1] << std::endl;
	std::cout << edge1[2] << std::endl;
	std::cout << edge2[0] << std::endl;
	std::cout << edge2[1] << std::endl;
	std::cout << edge2[2] << std::endl;
}

void baseballVertEdgeCalculator(float Center[3], float edge1[3], float edge2[3]){
	
	float range = sqrt(Center[1]*Center[1] + Center[2]*Center[2]);
	float cosTheta = Center[1]/range;
	float sinTheta = Center[2]/range;
	float radius = 0.0375; //Radius of baseball in meters
	
	if(Center[0]>=0){ //If equal to, both statements apply
		
		edge1[0] = Center[0];
		edge1[1] = Center[1]-radius*sinTheta; //Closer Edge (y closer to zero) (z is larger) (relative to center point);
    		edge1[2] = Center[2]+radius*cosTheta;
    
    		edge2[0] = Center[0]; 
    		edge2[1] = Center[1]+radius*sinTheta; //Farther Edge (y is farther from zero) (z is smaller) (relative to center point)
    		edge2[2] = Center[2]-radius*cosTheta;
	
	}
	else{
		edge1[0] = Center[0]; 
		edge1[1] = Center[1]+radius*sinTheta; //Closer Edge (y closer to zero) (z is larger) (relative to center point)
    		edge1[2] = Center[2]-radius*cosTheta;
    
    		edge2[0] = Center[0]; 
    		edge2[1] = Center[1]-radius*sinTheta; //Farther Edge (y is farther from zero) (z is smaller) (relative to center point)
    		edge2[2] = Center[2]+radius*cosTheta;
	}
	std::cout << edge1[0] << std::endl;
	std::cout << edge1[1] << std::endl;
	std::cout << edge1[2] << std::endl;
	std::cout << edge2[0] << std::endl;
	std::cout << edge2[1] << std::endl;
	std::cout << edge2[2] << std::endl;
}

int main(){
    Camera cam;	//Create Camera Object
    cam.enableStreams(848, 480, 90);	//Specify stream parameters
    
    float Center[3]; //Define the location of the center of the ball
    std::cout << "Enter x position of center: ";	//Give user time to set up the ball
    std::cin >> Center[0];
    std::cout << "Enter y position of center: ";
    std::cin >> Center[1];
    std::cout << "Enter z position of center: ";
    std::cin >> Center[2];
    //Note that the origin is considered to be the left imager (facing outwards) x axis facing right, y axis facing down, z axis facing away
    
    std::vector<rs2::frameset> frames;
    frames = cam.recordRSFrames(0,0);	//(n,m) Record and save 'n' frames for measurements throw out first 'm' frames
    auto intrin = cam.getIntrin(); //retreives the intrinsics of the camera necessary for deprojection
    
    //Horizontal Calculations
    
    float pixel1[2]; //Empty array of right edge
    float pixel2[2]; //Empty array of left edge
    float edge1[3]; //Closer Horiz Edge 
    float edge2[3]; //Farther Horiz Edge
    
    baseballHorizEdgeCalculator(Center,edge1,edge2);
    
    rs2_project_point_to_pixel(pixel1, &intrin, edge1); //Convert the distance and 2D pixel location into 3D position
    rs2_project_point_to_pixel(pixel2, &intrin, edge2); //Convert the distance and 2D pixel location into 3D position
    
    float center[2] = {(pixel1[0]+pixel2[0])/2.0f,(pixel1[1]+pixel2[1])/2.0f};
    float radiusX = (pixel1[0]-pixel2[0])/2;
    
    std::cout<<"Center: " << center[0] << " " << center[1] << std::endl;
    std::cout<<"Radius X " << radiusX << std::endl;
    
    //Vertical Calculations
    
    float pixel3[2];
    float pixel4[3];
    float edge3[3]; //Closer Vert edge
    float edge4[3]; //Farther Vert Edge
    
    baseballVertEdgeCalculator(Center,edge3,edge4);
    
    rs2_project_point_to_pixel(pixel3, &intrin, edge3); //Convert the distance and 2D pixel location into 3D position
    rs2_project_point_to_pixel(pixel4, &intrin, edge4); //Convert the distance and 2D pixel location into 3D position
    
    center[0] = (pixel3[0]+pixel4[0])/2.0f;
    center[1] = (pixel3[1]+pixel4[1])/2.0f;
    float radiusY = (pixel3[1]-pixel4[1])/2;
    
    std::cout<<"Center: " << center[0] << " " << center[1] << std::endl;
    std::cout<<"Radius Y " << radiusY << std::endl;
  	
    return 0;
}

