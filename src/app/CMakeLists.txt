# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

add_executable(sealtk
  About.cpp
  Main.cpp
  Panel.cpp
  Player.cpp
  PlayerTree.cpp
  PlayerViewer.cpp
  SplitterWindow.cpp
  Window.cpp

  glsl/PlayerViewer.qrc
  ${SEALTK_RESOURCE_FILE}
  )

target_link_libraries(sealtk
  SEALTKLib
  qtExtensions
  Qt5::Widgets
  )
target_include_directories(sealtk PRIVATE .)

set_property(TARGET sealtk PROPERTY RUNTIME_OUTPUT_DIRECTORY
  "${CMAKE_BINARY_DIR}/bin")

if(DEFINED SEALTK_EXECUTABLE_NAME)
  set_property(TARGET sealtk PROPERTY OUTPUT_NAME "${SEALTK_EXECUTABLE_NAME}")
endif()
