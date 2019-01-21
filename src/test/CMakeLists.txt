# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include (SEALTKUtils)

set(SEALTK_TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")

configure_file(
  "${PROJECT_SOURCE_DIR}/include/sealtk/test/Config.h.in"
  "${PROJECT_BINARY_DIR}/include/sealtk/test/Config.h"
  )

sealtk_add_library(sealtk::test
  SOURCES
    TestCore.cpp

  HEADERS
    TestCore.hpp

  PUBLIC_LINK_LIBRARIES
    Qt5::Core
    Qt5::Test
  )

function(sealtk_add_test name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} sealtk::test)
  qt5_discover_tests(${name} ${name})
endfunction()

sealtk_add_test(VideoSource)
target_link_libraries(VideoSource
  sealtk::core
  kwiver::vital_algo
  )
