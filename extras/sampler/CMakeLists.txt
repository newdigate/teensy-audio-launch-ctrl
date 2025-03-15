cmake_minimum_required(VERSION 3.10)
project(sampler)
set(CMAKE_CXX_STANDARD 17)
include(opengl.cmake)

include_directories(../../src)

add_definitions(-fms-extensions)
add_definitions(-DArduino=146)

include_directories(/usr/local/include ) #for lib soundio
add_definitions(-DCMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_executable(sampler sampler_main.cpp)

target_link_libraries(sampler teensy_x86_stubs)
target_link_libraries(sampler teensy_x86_sd_stubs)
target_link_libraries(sampler teensy_x86_bounce2_stubs)
target_link_libraries(sampler teensy_x86_encoder_stubs)
target_link_libraries(sampler teensy_audio_x86_stubs)
target_link_libraries(sampler teensy_audio_x86_stubs_soundio)
target_link_libraries(sampler rtmidi)
target_link_libraries(sampler teensy_variable_playback)
target_link_libraries(sampler teensy_x86_midi_stubs)
target_link_libraries(sampler teensy_x86_midi_stubs_rtmidi)
target_link_libraries(sampler teensy_st7735_linux_stubs)
target_link_libraries(sampler teensy_st7735_linux_opengl)
target_link_libraries(sampler teensy_x86_bounce2_stubs)
target_link_libraries(sampler teensy_gfx)
target_link_libraries(sampler teensy_gfx_controls)
target_link_libraries(sampler teensy_polyphony)
target_link_libraries(sampler arduino_tftpianoview)

target_link_libraries(sampler ${LIBS})

target_link_libraries(sampler "-L/usr/local/lib -lsoundio")
if  (APPLE)
        target_link_libraries(sampler
                "-framework CoreServices"
                "-framework CoreAudio"
                "-framework Foundation"
                "-framework AudioUnit")
endif()