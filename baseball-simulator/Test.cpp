//Test Program to fully utilize the hardware (without spin estimation)
//
//MainTestingRoutine.cpp
//
//JEY Solutions
//
//Created By: Yoni Arieh
//5/20/21
//

//Standard Includes
#include <stdio.h>
#include <iostream>

//Custom JEY Solutions headers
#include "BaseballSimulator.h"
#include "Camera.h"
#include "ImageData.h"
#include "CameraCalibration.h"
#include "DataStructure.h"
#include "LocPredictor.h"
#include "Tracker.h"
#include "ThresholdFilter.h"
#include "Visualizer.h"
#include "DataStructure.h"
#include "sendPost.hpp"

using namespace std;


int main(){

    DataStructure data;
    vector<float> pos1{1,2,3};
    vector<float> pos2{7,2,3};
    vector<float> pos3{6,2,3};
    vector<float> pos4{3,2,3};
    vector<float> pos5{4,2,3};
    data.positions.push_back(pos1);
    data.positions.push_back(pos2);
    data.positions.push_back(pos3);
    data.positions.push_back(pos4);
    data.positions.push_back(pos5);

    vector<float> error1{1,2,3};
    vector<float> error2{4,2,3};
    vector<float> error3{6,2,3};
    vector<float> error4{8,2,3};
    vector<float> error5{9,2,3};
    data.error.push_back(error1);
    data.error.push_back(error2);
    data.error.push_back(error3);
    data.error.push_back(error4);
    data.error.push_back(error5);

    data.timestamps.push_back(123333);
    data.timestamps.push_back(123334);
    data.timestamps.push_back(123335);
    data.timestamps.push_back(123336);
    data.timestamps.push_back(123337);

    data.username = "LiamSmallPeni";
    data.serialNumber = 1234;

    

    //****************************Spin Block***************************//
    
    //Deploy user input to input spin
    float spin[3];
    std::cout<< "Enter Wb, Ws, Wg in order:" << std::endl;
    std::cin >> spin[0] >> spin[1] >> spin[2];
    std::cout << std::endl << "Spin values set: " << spin[0] << std::endl << spin[1] << std::endl << spin[2] << std::endl;
    data.spin.push_back(spin[0]);
    data.spin.push_back(spin[1]);
    data.spin.push_back(spin[2]);
    
    //************************Populate Data Structure Block***********************//
    
    //We have already filled in position, error, and spin at this point
    //Add Camera serial number and a temoprary pitcher name to the data structure
    
    
    //****************************JSONify data block***************************//
    //Jsonify the data using the jsonify method in DataStructure
    
    //****************************HTTP Post block***************************//
    //Pass the JSON object to a POST object that will send the pitch to be recorded in the backend API
    sendPost httpPost;
    httpPost.sendRequest(data.jsonify());
    
    //****************************Completion Message***************************//
    std::cout<<"The Routine has completed" << std::endl;
}

