#
# (C) Copyright 2019-2024 Johns Hopkins University (JHU), All Rights Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---

cmake_minimum_required (VERSION 3.10)
project (sawSocketStreamer VERSION 1.2.0)

# create a list of required cisst libraries
set (REQUIRED_CISST_LIBRARIES cisstCommon
                              cisstVector
                              cisstOSAbstraction
                              cisstMultiTask
                              cisstParameterTypes)

find_package (cisst 1.2.1 REQUIRED ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND_AS_REQUIRED)

  # load cisst configuration
  include (${CISST_USE_FILE})

  # catkin/ROS paths
  cisst_set_output_path ()

  # create/configure file for find_package (sawSocketStreamer)
  set (sawSocketStreamer_INCLUDE_DIR
       "${sawSocketStreamer_SOURCE_DIR}/include"
       "${sawSocketStreamer_BINARY_DIR}/include")
  set (sawSocketStreamer_HEADER_DIR "${sawSocketStreamer_SOURCE_DIR}/include/sawSocketStreamer")
  set (sawSocketStreamer_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}")
  set (sawSocketStreamer_LIBRARIES sawSocketStreamer)

  # Allow c++ code to find local header files
  include_directories (${sawSocketStreamer_INCLUDE_DIR} ${sawSocketStreamer_BINARY_DIR})

  # add all config files for this component
  cisst_add_config_files (sawSocketStreamer)

  add_library (sawSocketStreamer
               ${sawSocketStreamer_HEADER_DIR}/sawSocketStreamerExport.h
               code/mtsSocketStreamer.cpp
               ${sawSocketStreamer_HEADER_DIR}/mtsSocketStreamer.h)
  set_target_properties (sawSocketStreamer PROPERTIES
                         VERSION ${sawSocketStreamer_VERSION}
                         FOLDER "sawSocketStreamer")
  cisst_target_link_libraries (sawSocketStreamer ${REQUIRED_CISST_LIBRARIES})

  # Install target for headers and library
  install (DIRECTORY
           "${sawSocketStreamer_SOURCE_DIR}/include/sawSocketStreamer"
           "${sawSocketStreamer_BINARY_DIR}/include/sawSocketStreamer"
           DESTINATION include
           COMPONENT sawSocketStreamer-dev)

  install (TARGETS sawSocketStreamer COMPONENT sawSocketStreamer
           RUNTIME DESTINATION bin
           LIBRARY DESTINATION lib
           ARCHIVE DESTINATION lib)

else (cisst_FOUND_AS_REQUIRED)
  message ("Information: code in ${CMAKE_CURRENT_SOURCE_DIR} will not be compiled, it requires ${REQUIRED_CISST_LIBRARIES}")
endif (cisst_FOUND_AS_REQUIRED)
