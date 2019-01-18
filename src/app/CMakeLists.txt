# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

add_executable(sealtk
  Main.cpp
  )

target_link_libraries(sealtk
  sealtk_gui
  qtExtensions
  Qt5::Widgets
  )

set_property(TARGET sealtk PROPERTY RUNTIME_OUTPUT_DIRECTORY
  "${PROJECT_BINARY_DIR}/bin")

if(DEFINED SEALTK_EXECUTABLE_NAME)
  set_property(TARGET sealtk PROPERTY OUTPUT_NAME "${SEALTK_EXECUTABLE_NAME}")
endif()
