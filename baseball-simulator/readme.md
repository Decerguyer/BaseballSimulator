# To run any of the baseball scripts:
1. Pull the desired script from the folder
2. Change the cpp file listed in CMake.txts as appropriate
3. Using terminal, navigate to baseball-simulator folder in librealsense/build/examples
4. Type make into the command line and press enter
5. Run the executable



# CMAKE example shown below:
The cpp file name should be renamed

# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2019 Intel Corporation. All Rights Reserved.
#  minimum required cmake version: 3.1.0
cmake_minimum_required(VERSION 3.1.0)

project(RealsenseExamplesBaseballSimulator)

find_package( OpenCV REQUIRED )
add_executable(rs-baseball-simulator rs-baseball-simulator.cpp)
set_property(TARGET rs-baseball-simulator PROPERTY CXX_STANDARD 11)
include_directories(rs-baseball-simulator ${OpenCV_INCLUDE_DIRS})
target_link_libraries(rs-baseball-simulator ${DEPENDENCIES} ${OpenCV_LIBS})

set_target_properties (rs-baseball-simulator PROPERTIES FOLDER "Examples")
install(TARGETS rs-baseball-simulator RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
