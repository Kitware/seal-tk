# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(SEALTKUtils)

sealtk_add_library(sealtk::noaa_core
  SOURCES
    FilenameUtils.cpp
    ImageListVideoSourceFactory.cpp

  HEADERS
    FilenameUtils.hpp
    ImageListVideoSourceFactory.hpp

  PUBLIC_LINK_LIBRARIES
    sealtk::core
    qtExtensions
    Qt5::Core
  )
