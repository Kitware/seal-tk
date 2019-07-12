# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(SEALTKUtils)

macro(get_absdir var val)
  if(IS_ABSOLUTE "${val}")
    set(${var} "${val}")
  else()
    set(${var} "${CMAKE_INSTALL_PREFIX}/${val}")
  endif()
endmacro()

get_absdir(bindir "${CMAKE_INSTALL_BINDIR}")
get_absdir(datadir "${CMAKE_INSTALL_DATADIR}")
file(RELATIVE_PATH SEALTK_NOAA_RELATIVE_SHARE_DIR "${bindir}" "${datadir}")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/Config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/Config.h"
  )

sealtk_add_library(sealtk::noaa_core
  SOURCES
    FilenameUtils.cpp
    ImageListVideoSourceFactory.cpp

  HEADERS
    FilenameUtils.hpp
    ImageListVideoSourceFactory.hpp
    "${CMAKE_CURRENT_BINARY_DIR}/Config.h"

  PUBLIC_LINK_LIBRARIES
    sealtk::core
    qtExtensions
    Qt5::Core

  EXPORT_HEADER Export.h
  )
