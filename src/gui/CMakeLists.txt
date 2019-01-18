# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

sealtk_add_library(gui
  SOURCES
    About.cpp
    Panel.cpp
    Player.cpp
    PlayerTree.cpp
    PlayerViewer.cpp
    Resources.cpp
    SplitterWindow.cpp
    Window.cpp

    glsl/PlayerViewer.qrc
    ${SEALTK_RESOURCE_FILE}

  HEADERS
    About.hpp
    Panel.hpp
    Player.hpp
    PlayerTree.hpp
    PlayerViewer.hpp
    Resources.hpp
    SplitterWindow.hpp
    Window.hpp
  )
target_link_libraries(sealtk_gui
  sealtk_core
  )
