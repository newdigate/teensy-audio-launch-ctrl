cmake_minimum_required(VERSION 3.10)
project(launch_ctrl)

set(CMAKE_CXX_STANDARD 14)
teensy_include_directories(../../src)
teensy_add_executable(launch_ctrl launch_ctrl.ino ../../src/sampleplaymidicontroller.cpp ../../src/polyphonic.cpp)
teensy_target_link_libraries(launch_ctrl Audio USBHost_t36 ST7735_t3  arm_math SD SdFat  SPI SerialFlash  Wire  cores )
target_link_libraries(launch_ctrl.elf m stdc++)