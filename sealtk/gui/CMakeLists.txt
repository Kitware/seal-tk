# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(SEALTKUtils)

sealtk_add_library(sealtk::gui
  SOURCES
    AbstractItemRepresentation.cpp
    ClassificationSummaryRepresentation.cpp
    CreateDetectionPlayerTool.cpp
    DetectionRepresentation.cpp
    FilterWidget.cpp
    FusionModel.cpp
    Player.cpp
    PlayerControl.cpp
    PlayerTool.cpp
    Resources.cpp
    SplitterWindow.cpp

    resources/Resources.qrc
    resources/Icons.qrc

  HEADERS
    AbstractItemRepresentation.hpp
    ClassificationSummaryRepresentation.hpp
    CreateDetectionPlayerTool.hpp
    DetectionRepresentation.hpp
    FilterWidget.hpp
    FusionModel.hpp
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
