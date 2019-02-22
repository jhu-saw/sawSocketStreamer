#
# (C) Copyright 2019 Johns Hopkins University (JHU), All Rights Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---

project (sawSocketStreamer)

cmake_minimum_required(VERSION 2.8)

set (REQUIRED_CISST_LIBRARIES
     cisstCommon
     cisstVector
     cisstOSAbstraction
     cisstMultiTask
     cisstParameterTypes)

find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND)
  # load cisst configuration
  include (${CISST_USE_FILE})

  # catkin/ROS paths
  cisst_is_catkin_build (sawSocketStreamer_IS_CATKIN_BUILT)
  if (sawSocketStreamer_IS_CATKIN_BUILT)
    set (LIBRARY_OUTPUT_PATH "${CATKIN_DEVEL_PREFIX}/lib")
  endif ()

  include_directories (${CMAKE_BINARY_DIR})

  set (sawSocketStreamer_INCLUDE_DIR
       "${sawSocketStreamer_SOURCE_DIR}/include")

  include_directories (${sawSocketStreamer_INCLUDE_DIR})

  add_library (sawSocketStreamer
               "include/sawSocketStreamer/sawSocketStreamerExport.h"
               "include/sawSocketStreamer/mtsSocketStreamer.h"
               "code/mtsSocketStreamer.cpp")

  cisst_target_link_libraries (sawSocketStreamer
                               ${REQUIRED_CISST_LIBRARIES})

endif (cisst_FOUND)
