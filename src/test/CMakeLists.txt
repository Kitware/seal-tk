# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

set(SEALTK_TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")

configure_file(
  "${PROJECT_SOURCE_DIR}/include/sealtk/test/Config.h.in"
  "${PROJECT_BINARY_DIR}/include/sealtk/test/Config.h"
  )

add_library(sealtk_test TestCore.cpp)
target_link_libraries(sealtk_test PUBLIC Qt5::Core Qt5::Test)

function(sealtk_add_test name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} sealtk_test)
  set(test_output_file "${CMAKE_CURRENT_BINARY_DIR}/test_${name}.cmake")
  add_custom_command(TARGET ${name}
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
      "-DTEST_NAME:STRING=${name}"
      "-DTEST_EXECUTABLE:FILEPATH=$<TARGET_FILE:${name}>"
      "-DTEST_OUTPUT_FILE:FILEPATH=${test_output_file}"
      -P "${PROJECT_SOURCE_DIR}/cmake/QtTestDiscoverTests.cmake"
    BYPRODUCTS "${test_output_file}"
    )
  set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES
    "${test_output_file}")
endfunction()

sealtk_add_test(VideoSource)
target_link_libraries(VideoSource
  sealtk_core
  kwiver::vital_algo
  )
