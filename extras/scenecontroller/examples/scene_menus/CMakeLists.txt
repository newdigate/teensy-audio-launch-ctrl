cmake_minimum_required(VERSION 3.10)
project(scene_menus)
set(CMAKE_CXX_STANDARD 17)

include_directories(../../../../src)

add_definitions(-fms-extensions)
add_definitions(-DArduino=146)

add_executable(scene_menus scene_menus.cpp)

target_link_libraries(scene_menus teensy_x86_stubs)
target_link_libraries(scene_menus teensy_x86_sd_stubs)
target_link_libraries(scene_menus teensy_x86_bounce2_stubs)
target_link_libraries(scene_menus teensy_x86_encoder_stubs)
target_link_libraries(scene_menus teensy_audio_x86_stubs)
target_link_libraries(scene_menus teensy_audio_x86_stubs_soundio)
target_link_libraries(scene_menus rtmidi)
target_link_libraries(scene_menus teensy_variable_playback)
target_link_libraries(scene_menus teensy_x86_midi_stubs)
target_link_libraries(scene_menus teensy_x86_midi_stubs_rtmidi)
target_link_libraries(scene_menus teensy_st7735_linux_stubs)
target_link_libraries(scene_menus teensy_st7735_linux_extras_opengl)
target_link_libraries(scene_menus teensy_x86_bounce2_stubs)
target_link_libraries(scene_menus teensy_gfx)
target_link_libraries(scene_menus teensy_gfx_controls)

include(opengl.cmake)
target_link_libraries(scene_menus ${LIBS})

target_link_libraries(scene_menus "-L/usr/local/lib -lsoundio")
if  (APPLE)
        target_link_libraries(scene_menus
                "-framework CoreServices"
                "-framework CoreAudio"
                "-framework Foundation"
                "-framework AudioUnit")
endif()