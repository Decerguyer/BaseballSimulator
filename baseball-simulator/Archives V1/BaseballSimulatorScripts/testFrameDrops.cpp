//  testFrameDrops.cpp
//  baseball-test
//
//  Created by Jordan Mayor on 12/20/20.
//

#include <stdio.h>
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <iostream>             // for cout
#include <vector>
#include <iomanip>


int main(int argc, char * argv[]) try
{
//    // First, create a rs2::context.
//    // The context represents the current platform with respect to connected devices
//    rs2::context ctx;
//
//    // Using the context we can get all connected devices in a device list
//    rs2::device_list devices = ctx.query_devices();
//
//    rs2::frame_queue q(5);
//    
//    rs2::frame_queue q1;
//    rs2::frame_queue q2;
//;
//    
//    std::vector<rs2::stream_profile> streamProfiles;
//    for (rs2::device dev : devices){
//        // Given a device, we can query its sensors using:
//        std::vector<rs2::sensor> sensors = dev.query_sensors();
//        
//        sensors[0].start(q1);
//        sensors[1].start(q2);
//        
//    }
//
    
    
    // Create a Pipeline - this serves as a top-level API for streaming and processing frames
    rs2::pipeline p;
        
    rs2::config cfg;
    
    int width = 848;
    int height = 480;
    int fps = 90;
    
    cfg.enable_stream(RS2_STREAM_DEPTH, width, height, RS2_FORMAT_Z16, fps);
    cfg.enable_stream(RS2_STREAM_INFRARED, 1, width, height, RS2_FORMAT_Y8, fps);
    //cfg.enable_stream(RS2_STREAM_INFRARED, 2, width, height, RS2_FORMAT_Y8, fps);
    
    rs2::pipeline_profile profile = p.start(cfg);
    
    std::vector<int> frameNums;
    std::vector<float> timeStamps;
    


    for (int i = 0; i < 1000; i++)
    {
        // Block program until frames arrive
        rs2::frameset frame = p.wait_for_frames();

        frameNums.push_back((int)frame.get_frame_number());
        timeStamps.push_back(frame.get_frame_metadata(RS2_FRAME_METADATA_SENSOR_TIMESTAMP)); 
    }
    p.stop();

    for (int i = 0; i < (int)frameNums.size() - 1; i++){
        std::cout << frameNums[i] << " ";
        
        std::cout << timeStamps[i + 1] - timeStamps[i] << std::endl;
    }
        
        
    return EXIT_SUCCESS;
}
catch (const rs2::error & e)
{
    std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}

