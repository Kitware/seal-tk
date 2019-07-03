# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(SEALTKUtils)

add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(kwiver)

sealtk_add_executable(sealtk::noaa_app
  SOURCES
    Main.cpp

  PUBLIC_LINK_LIBRARIES
    sealtk::noaa_core
    sealtk::noaa_gui

  TARGET_NAME_VAR name
  )

set_property(TARGET ${name} PROPERTY OUTPUT_NAME sealtk)

install(DIRECTORY share/seal-tk DESTINATION "${CMAKE_INSTALL_DATADIR}")
add_custom_target(noaa_share_files
  ${CMAKE_COMMAND}
    "-DDATA_BINARY_DIR=${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_DATADIR}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuildDirectoryDataFiles.cmake"
  )
add_dependencies(${name} noaa_share_files)

add_subdirectory(test)
