# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(SEALTKUtils)

sealtk_add_library(sealtk::noaa_core
  SOURCES
    FilenameUtils.cpp
    ImageListVideoSourceFactory.cpp
    Plugins.cpp

  HEADERS
    FilenameUtils.hpp
    ImageListVideoSourceFactory.hpp
    Plugins.hpp

  PUBLIC_LINK_LIBRARIES
    sealtk::core
    qtExtensions
    Qt5::Core
  )

sealtk_add_library(sealtk::noaa_gui
  SOURCES
    About.cpp
    Window.cpp
    SEALTKBranding.qrc

  HEADERS
    About.hpp
    Window.hpp

  PUBLIC_LINK_LIBRARIES
    sealtk::gui
    sealtk::noaa_core
    qtExtensions
    Qt5::Widgets
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

add_subdirectory(test)
