# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(SEALTKUtils)

sealtk_add_library(sealtk::gui
  SOURCES
    AbstractItemModel.cpp
    AbstractItemRepresentation.cpp
    Player.cpp
    PlayerControl.cpp
    Resources.cpp
    SplitterWindow.cpp

    resources/Resources.qrc

  HEADERS
    AbstractItemModel.hpp
    AbstractItemRepresentation.hpp
    Player.hpp
    PlayerControl.hpp
    Resources.hpp
    SplitterWindow.hpp

  PUBLIC_LINK_LIBRARIES
    sealtk::core

  PRIVATE_LINK_LIBRARIES
    Qt5::Concurrent

  EXPORT_HEADER Export.h
  )

add_subdirectory(test)
