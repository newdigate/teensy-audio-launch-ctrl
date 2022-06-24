cmake_minimum_required(VERSION 3.10)
project(menu_example2)
set(CMAKE_CXX_STANDARD 17)

find_package(teensy_x86_stubs)
include_directories(${teensy_x86_stubs_INCLUDE_DIR})

find_package(teensy_x86_encoder_stubs)
include_directories(${teensy_x86_encoder_stubs_INCLUDE_DIR})

find_package(teensy_x86_bounce2_stubs)
include_directories(${teensy_x86_bounce2_stubs_INCLUDE_DIR})

find_package(teensy_audio_x86_stubs)
include_directories(${teensy_audio_x86_stubs_INCLUDE_DIR})

find_package(teensy_audio_x86_stubs_soundio)
include_directories(${teensy_audio_x86_stubs_soundio_INCLUDE_DIR})

find_package(teensy_audio_x86_stubs)
include_directories(${teensy_audio_x86_stubs_INCLUDE_DIR})

find_package(teensy_x86_sd_stubs)
include_directories(${teensy_x86_sd_stubs_INCLUDE_DIR})

find_package(teensy_x86_midi_stubs)
include_directories(${teensy_x86_midi_stubs_INCLUDE_DIR})

find_path(RTMIDI_INCLUDE_DIR RtMidi.h
  PATHS
    /usr/local/include
    /usr/include
    /usr/include/rtmidi)
include_directories(${RTMIDI_INCLUDE_DIR})

find_package(teensy_x86_midi_stubs_rtmidi)
include_directories(${teensy_x86_midi_stubs_rtmidi_INCLUDE_DIR})

find_package(teensy_st7735_linux_stubs)
include_directories(${teensy_st7735_linux_stubs_INCLUDE_DIR})

find_package(teensy_st7735_linux_opengl)
include_directories(${teensy_st7735_linux_extras_opengl_INCLUDE_DIR})

include_directories(../../src)
include_directories(../ILI9341_t3_Menu)

add_definitions(-fms-extensions)
add_definitions(-DArduino=146)

add_executable(menu_example2 menu_example2.cpp)

target_link_libraries(menu_example2 ${teensy_x86_stubs_LIBS})
target_link_libraries(menu_example2 ${teensy_x86_sd_stubs_LIBS})
target_link_libraries(menu_example2 ${teensy_x86_bounce2_stubs_LIBS})
target_link_libraries(menu_example2 ${teensy_x86_encoder_stubs_LIBS})
target_link_libraries(menu_example2 ${teensy_audio_x86_stubs_LIBS})
target_link_libraries(menu_example2 ${teensy_audio_x86_stubs_soundio_LIBS})
target_link_libraries(menu_example2 -L/usr/local/lib/ rtmidi)
target_link_libraries(menu_example2 ${teensy_variable_playback_LIBS})
target_link_libraries(menu_example2 ${teensy_x86_midi_stubs_LIBS})
target_link_libraries(menu_example2 ${teensy_x86_midi_stubs_rtmidi_LIBS})
target_link_libraries(menu_example2 ${teensy_st7735_linux_stubs_LIBS})
target_link_libraries(menu_example2 ${teensy_st7735_linux_extras_opengl_LIBS})
target_link_libraries(menu_example2 ${teensy_x86_bounce2_stubs_LIBS})

include(opengl.cmake)
target_link_libraries(menu_example2 ILI9341_t3_Menu ${LIBS})

target_link_libraries(menu_example2 "-L/usr/local/lib -lsoundio")
if  (APPLE)
        target_link_libraries(menu_example2
                "-framework CoreServices"
                "-framework CoreAudio"
                "-framework Foundation"
                "-framework AudioUnit")
endif()