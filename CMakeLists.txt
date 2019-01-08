# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

cmake_minimum_required(VERSION 3.12)
project(seal-tk
  VERSION 0.1
  LANGUAGES CXX
  )

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC 1)
set(CMAKE_AUTORCC 1)
set(CMAKE_AUTOUIC 1)

set(SEALTK_BRANDING SEAL-TK CACHE LIST "Branding to use")
set_property(CACHE SEALTK_BRANDING PROPERTY STRINGS
  SEAL-TK
  Custom
  )

include("${CMAKE_SOURCE_DIR}/branding/${SEALTK_BRANDING}.cmake")

find_package(Qt5 REQUIRED COMPONENTS
  Widgets
  Xml
  )
find_package(qtExtensions REQUIRED)

add_executable(sealtk
  src/About.cpp
  src/Main.cpp
  src/Panel.cpp
  src/Player.cpp
  src/PlayerTree.cpp
  src/PlayerViewer.cpp
  src/SplitterWindow.cpp
  src/Window.cpp

  src/glsl/PlayerViewer.qrc
  )

target_link_libraries(sealtk
  qtExtensions
  Qt5::Widgets
  )
target_include_directories(sealtk PRIVATE src)

set_property(TARGET sealtk PROPERTY RUNTIME_OUTPUT_DIRECTORY bin)

if(DEFINED SEALTK_EXECUTABLE_NAME)
  set_property(TARGET sealtk PROPERTY OUTPUT_NAME ${SEALTK_EXECUTABLE_NAME})
endif()

find_program(GIT_EXECUTABLE git)
if(GIT_EXECUTABLE)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --dirty --always
    OUTPUT_VARIABLE SEALTK_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
endif()

if(NOT SEALTK_VERSION)
  set(SEALTK_VERSION "v${PROJECT_VERSION}")
endif()

configure_file(
  "${CMAKE_SOURCE_DIR}/src/Config.h.in"
  "${CMAKE_BINARY_DIR}/src/Config.h"
  )
include_directories("${CMAKE_BINARY_DIR}/src")

enable_testing()

find_package(Python3)

if(TARGET Python3::Interpreter)
  if(GIT_EXECUTABLE)
    set(git_args --git ${GIT_EXECUTABLE})
  endif()
  add_test(
    NAME SourceCheck
    COMMAND Python3::Interpreter
    "${CMAKE_SOURCE_DIR}/utilities/source_check.py" ${git_args}
    )
endif()
