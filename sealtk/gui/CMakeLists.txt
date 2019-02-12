# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(SEALTKUtils)

sealtk_add_library(sealtk::gui
  SOURCES
    Player.cpp
    PlayerControl.cpp
    Resources.cpp
    SplitterWindow.cpp

    glsl/PlayerViewer.qrc

  HEADERS
    Player.hpp
    PlayerControl.hpp
    Resources.hpp
    SplitterWindow.hpp

  PUBLIC_LINK_LIBRARIES
    sealtk::core

  EXPORT_HEADER Export.h
  )
