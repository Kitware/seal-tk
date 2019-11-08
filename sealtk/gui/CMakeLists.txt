# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(SEALTKUtils)

sealtk_add_library(sealtk::gui
  SOURCES
    AbstractItemRepresentation.cpp
    CreateDetectionPlayerTool.cpp
    DetectionRepresentation.cpp
    FusionModel.cpp
    GlobInputDialog.cpp
    Player.cpp
    PlayerControl.cpp
    PlayerTool.cpp
    Resources.cpp
    SplitterWindow.cpp

    resources/Resources.qrc

  HEADERS
    AbstractItemRepresentation.hpp
    CreateDetectionPlayerTool.hpp
    DetectionRepresentation.hpp
    FusionModel.hpp
    GlobInputDialog.hpp
    Player.hpp
    PlayerControl.hpp
    PlayerTool.hpp
    Resources.hpp
    SplitterWindow.hpp

  PUBLIC_LINK_LIBRARIES
    sealtk::core

  PRIVATE_LINK_LIBRARIES
    Qt5::Concurrent

  EXPORT_HEADER Export.h
  )

add_subdirectory(test)
