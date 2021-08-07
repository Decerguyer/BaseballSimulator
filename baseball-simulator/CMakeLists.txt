# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2019 Intel Corporation. All Rights Reserved.
#  minimum required cmake version: 3.1.0
cmake_minimum_required(VERSION 3.1.0)

project(RealsenseExamplesBaseballSimulator)

include(FindPkgConfig)
pkg_check_modules(CURLPP REQUIRED curlpp)

find_package( OpenCV REQUIRED )
find_library(REALSENSE2_FOUND realsense2 HINTS ${LIBRARY_DIR} REQUIRED)

#Add in FeatureAnalysis.cpp when it is ready to be included
add_executable(rs-baseball-simulator Camera.cpp D400.cpp DataStructure.cpp ImageData.cpp ImageReconditioning.cpp LocPredictor.cpp PseudoTracker.cpp sendPost.cpp ThresholdFilter.cpp Tracker.cpp Visualizer.cpp BaseballSimulator.cpp main.cpp)
set_property(TARGET rs-baseball-simulator PROPERTY CXX_STANDARD 11)
include_directories(rs-baseball-simulator ${OpenCV_INCLUDE_DIRS})
target_link_libraries(rs-baseball-simulator ${DEPENDENCIES} ${OpenCV_LIBS} ${REALSENSE2_FOUND} ${CURLPP_LDFLAGS})
set_target_properties (rs-baseball-simulator PROPERTIES FOLDER "Examples")
install(TARGETS rs-baseball-simulator RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

