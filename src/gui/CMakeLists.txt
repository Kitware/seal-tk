# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(SEALTKUtils)

sealtk_add_library(sealtk::gui
  SOURCES
    Panel.cpp
    Player.cpp
    PlayerTree.cpp
    PlayerViewer.cpp
    Resources.cpp
    SplitterWindow.cpp
    Window.cpp

    glsl/PlayerViewer.qrc

  HEADERS
    Panel.hpp
    Player.hpp
    PlayerTree.hpp
    PlayerViewer.hpp
    Resources.hpp
    SplitterWindow.hpp
    Window.hpp

  PUBLIC_LINK_LIBRARIES
    sealtk::core
  )
