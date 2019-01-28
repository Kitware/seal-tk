# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(SEALTKUtils)

sealtk_add_kwiver_plugin(sealtk::noaa_kwiver
  SOURCES
    TimestampPassthrough.cpp
    Plugin.cpp

  HEADERS
    TimestampPassthrough.hpp

  PUBLIC_LINK_LIBRARIES
    sealtk::core
    sealtk::noaa_core
    kwiver::vital
    kwiver::vital_algo
    Qt5::Core
  )
