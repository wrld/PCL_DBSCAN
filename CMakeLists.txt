cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(PCL_DBSCAN)

find_package(PCL 1.5 REQUIRED)# REQUIRED COMPONENTS io registration segmentation visualization)

include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})
 message(STATUS "PCL status:")
    message(STATUS "    version: ${PCL_VERSION}")
    message(STATUS "    directory: ${PCL_DIR}")
add_executable (PCL_DBSCAN DBSCAN.cpp)
target_link_libraries (PCL_DBSCAN ${PCL_LIBRARIES})

