cmake_minimum_required(VERSION 3.5)
project(teensy_audio_launch_ctrl C CXX)
set(teensy_audio_launch_ctrl_VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 14)
set(SOURCE_FILES
        polyphonic.cpp)

set(HEADER_FILES
        polyphonic.h
        extracolors.h
        TeensyAudioLaunchCtrl.h)

if (NOT DEFINED TEENSY_VERSION)
        SET(CMAKE_VERBOSE_MAKEFILE 1)
        include(CMakeLists.linux.cmake.in)
else()
    teensy_add_library(
            teensy_audio_launch_ctrl
            ${SOURCE_FILES}
    )
endif()