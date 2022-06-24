cmake_minimum_required(VERSION 3.10)
project(teensy_scene_controller)
set(CMAKE_CXX_STANDARD 11)

find_package(teensy_x86_stubs)
include_directories(${teensy_x86_stubs_INCLUDE_DIR})

find_package(teensy_st7735_linux_stubs)
include_directories(${teensy_st7735_linux_stubs_INCLUDE_DIR})

add_definitions(-DARDUINO=143)
add_library(teensy_scene_controller
    scenecontroller.cpp
              )