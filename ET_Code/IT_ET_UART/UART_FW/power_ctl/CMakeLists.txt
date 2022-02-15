cmake_minimum_required(VERSION 2.8.3)

include_directories(${ROS_LIB_DIR})
project(pwr_ctl_fw)

FILE(GLOB_RECURSE ros_src
    "${ROS_LIB_DIR}/*.cpp"
    "${ROS_LIB_DIR}/*.h")
#Change this to wherever minicore got installed
set(ARDUINO_SDK_PATH "~/.arduino15/.arduino15/packages/MiniCore/")

set(ARDUINO_CMAKE_AVRDUDE_PROGRAM $ENV{ARDUINO_SDK_PATH}/hardware/tools/avr/bin/avrdude)
add_library(ros_lib ${ros_src})

message("Board list:")
print_board_list()

message("Platforms: " ${ARDUINO_PLATFORMS})
message("power_ctl subdir: Using SDK path: " ${ARDUINO_SDK_PATH})

generate_arduino_firmware(pwr_ctl_fw
  SRCS pwr_control.cpp  ${ROS_LIB_DIR}/time.cpp
  BOARD uno
  PORT /dev/ttyACM0
)

target_link_libraries(pwr_ctl_fw ros_lib)
