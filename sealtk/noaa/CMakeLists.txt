# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(SEALTKUtils)

add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(kwiver)

set(NOAA_TRACK_READER "kw18" CACHE STRING
    "Name of KWIVER plugin used by the NOAA GUI to read tracks")
set(NOAA_TRACK_WRITER "kw18" CACHE STRING
    "Name of KWIVER plugin used by the NOAA GUI to write tracks")
set(NOAA_VIDEO_READER "noaa_timestamp_passthrough" CACHE STRING
    "Name of KWIVER plugin used by the NOAA GUI to read video")
set(NOAA_VIDEO_READER_PASSTHROUGH "vxl" CACHE STRING
    "Name of pass-through KWIVER plugin used by the NOAA GUI to read video")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/PluginConfig.hpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/PluginConfig.hpp
  @ONLY
  )

sealtk_add_executable(sealtk::noaa_app
  SOURCES
    Main.cpp

  PUBLIC_LINK_LIBRARIES
    sealtk::noaa_core
    sealtk::noaa_gui

  TARGET_NAME_VAR name
  )

set_property(TARGET ${name} PROPERTY OUTPUT_NAME sealtk)

sealtk_add_data(noaa_data_files
  FILES
    seal-tk/pipelines/test.pipe

  FROM share
  )
add_dependencies(${name} noaa_data_files)

add_subdirectory(test)
