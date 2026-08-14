#
# (C) Copyright 2019-2026 Johns Hopkins University (JHU), All Rights Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---

cmake_minimum_required (VERSION 3.16)
project (sawSocketStreamerCore VERSION 1.3.0)

find_package (cisst REQUIRED)
cisst_cpack_settings (
  VENDOR "JHU"
  MAINTAINER "anton.deguet@jhu.edu")

add_subdirectory (components)

include (CPack)
cpack_add_component (sawSocketStreamer)
cpack_add_component (sawSocketStreamer-dev
                     DEPENDS sawSocketStreamer)
