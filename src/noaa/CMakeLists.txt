# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

sealtk_add_library(sealtk::noaa
  SOURCES
    About.cpp
    Window.cpp
    SEALTKBranding.qrc

  HEADERS
    About.hpp
    Window.hpp

  PUBLIC_LINK_LIBRARIES
    sealtk::gui
    qtExtensions
    Qt5::Widgets
  )

sealtk_add_executable(sealtk::noaa_app
  SOURCES
    Main.cpp

  PUBLIC_LINK_LIBRARIES
    sealtk::noaa
  )

set_property(TARGET sealtk_noaa_app PROPERTY OUTPUT_NAME sealtk)
