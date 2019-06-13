# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(SEALTKUtils)

set(SEALTK_NOAA_CONFIG_IN
  "${PROJECT_SOURCE_DIR}/sealtk/noaa/core/Config.h.in")
set(SEALTK_NOAA_CONFIG_OUT
  "${PROJECT_BINARY_DIR}/sealtk/noaa/core/Config.h")

macro(get_absdir var val)
  if(IS_ABSOLUTE "${val}")
    set(${var} "${val}")
  elseif(WIN32)
    set(${var} "C:/${val}")
  else()
    set(${var} "/${val}")
  endif()
endmacro()

get_absdir(bindir "${CMAKE_INSTALL_BINDIR}")
get_absdir(datadir "${CMAKE_INSTALL_DATADIR}")
file(RELATIVE_PATH SEALTK_NOAA_RELATIVE_SHARE_DIR "${bindir}" "${datadir}")

configure_file(
  "${SEALTK_NOAA_CONFIG_IN}"
  "${SEALTK_NOAA_CONFIG_OUT}"
  )

sealtk_add_library(sealtk::noaa_core
  SOURCES
    FilenameUtils.cpp
    ImageListVideoSourceFactory.cpp

  HEADERS
    FilenameUtils.hpp
    ImageListVideoSourceFactory.hpp
    "${SEALTK_NOAA_CONFIG_OUT}"

  PUBLIC_LINK_LIBRARIES
    sealtk::core
    qtExtensions
    Qt5::Core

  EXPORT_HEADER Export.h
  )
