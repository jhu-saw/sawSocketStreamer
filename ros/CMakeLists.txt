#
# (C) Copyright 2026 Johns Hopkins University (JHU), All Rights Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---

cmake_minimum_required (VERSION 3.16)
project (saw_socket_streamer_ros VERSION 1.3.0)

find_package (ament_cmake REQUIRED)
find_package (ament_cmake_python REQUIRED)

set (ROS2_depends
     rclcpp std_msgs std_srvs geometry_msgs sensor_msgs diagnostic_msgs
     tf2_ros tf2_msgs
     cisst_msgs crtk_msgs cisst_ros_bridge cisst_ros_crtk)
foreach (_dep ${ROS2_depends})
  find_package (${_dep} REQUIRED)
endforeach ()

set (REQUIRED_CISST_LIBRARIES
     cisstCommon
     cisstVector
     cisstOSAbstraction
     cisstMultiTask
     cisstParameterTypes)

find_package (cisst 1.2.1 REQUIRED COMPONENTS ${REQUIRED_CISST_LIBRARIES})

cisst_set_output_path ()

find_package (sawSocketStreamer 1.2.0)

if (sawSocketStreamer_FOUND)

  include_directories (${sawSocketStreamer_INCLUDE_DIR})
  link_directories (${sawSocketStreamer_LIBRARY_DIR})

  add_executable (
    socket_streamer_ros_bridge
    src/socket_streamer_ros_bridge.cpp)

  target_link_libraries (
    socket_streamer_ros_bridge
    ${sawSocketStreamer_LIBRARIES})

  target_link_libraries (socket_streamer_ros_bridge ${cisst_LIBRARIES})

  ament_target_dependencies (
    socket_streamer_ros_bridge
    ${ROS2_depends})

  install (
    TARGETS socket_streamer_ros_bridge
    RUNTIME DESTINATION lib/${PROJECT_NAME})

  install (
    PROGRAMS scripts/demo_js.py
    DESTINATION lib/${PROJECT_NAME}
    RENAME demo_js)

  install (
    DIRECTORY launch share
    DESTINATION share/${PROJECT_NAME})

else (sawSocketStreamer_FOUND)
  message ("Information: code in ${CMAKE_CURRENT_SOURCE_DIR} will not be compiled, it requires sawSocketStreamer")
endif (sawSocketStreamer_FOUND)


ament_package ()
