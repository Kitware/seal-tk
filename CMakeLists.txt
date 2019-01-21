# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

cmake_minimum_required(VERSION 3.12)
project(seal-tk
  VERSION 0.1
  LANGUAGES CXX
  )

include_directories(
  "${PROJECT_SOURCE_DIR}/include"
  "${PROJECT_BINARY_DIR}/include"
  )

list(APPEND CMAKE_MODULE_PATH
  "${PROJECT_SOURCE_DIR}/cmake"
  "${PROJECT_SOURCE_DIR}/cmake/thirdparty"
  )

find_package(Qt5 REQUIRED COMPONENTS
  Test
  Widgets
  Xml
  )
find_package(qtExtensions REQUIRED)
find_package(kwiver REQUIRED)
find_package(Git)
find_package(Python3)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC 1)
set(CMAKE_AUTORCC 1)
set(CMAKE_AUTOUIC 1)

set(SEALTK_BRANDING SEAL-TK CACHE LIST "Branding to use")
set_property(CACHE SEALTK_BRANDING PROPERTY STRINGS
  SEAL-TK
  Custom
  )

include("${PROJECT_SOURCE_DIR}/branding/${SEALTK_BRANDING}.cmake")

enable_testing()

if(TARGET Python3::Interpreter)
  if(TARGET Git::Git)
    set(git_args --git $<TARGET_FILE:Git::Git>)
  endif()
  add_test(
    NAME SourceCheck
    COMMAND Python3::Interpreter
    "${PROJECT_SOURCE_DIR}/utilities/source_check.py" ${git_args}
    )
endif()

add_subdirectory(src)
