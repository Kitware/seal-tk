# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

function(qt5_discover_tests test_name target)
  set(test_output_file "${CMAKE_CURRENT_BINARY_DIR}/${test_name}.cmake")
  add_custom_command(TARGET ${target}
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
      "-DTEST_NAME:STRING=${test_name}"
      "-DTEST_EXECUTABLE:FILEPATH=$<TARGET_FILE:${target}>"
      "-DTEST_OUTPUT_FILE:FILEPATH=${test_output_file}"
      -P "${PROJECT_SOURCE_DIR}/cmake/QtTestDiscoverTests.cmake"
    BYPRODUCTS "${test_output_file}"
    )
  set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES
    "${test_output_file}")
endfunction()

function(sealtk_parse_name name suffix_var)
  if(NOT name MATCHES "^sealtk::([a-zA-Z0-9_]+)$")
    message(FATAL_ERROR "sealtk_add_library() name must be of the form "
      "sealtk::<name>")
  endif()
  set(${suffix_var} "${CMAKE_MATCH_1}" PARENT_SCOPE)
endfunction()

function(sealtk_add_library name)
  sealtk_parse_name(${name} suffix)

  set(sal_multi
    SOURCES
    HEADERS
    PRIVATE_LINK_LIBRARIES
    PUBLIC_LINK_LIBRARIES
    INTERFACE_LINK_LIBRARIES
    )
  cmake_parse_arguments(sal "" "" "${sal_multi}" ${ARGN})

  set(sal_headers)
  foreach(h ${sal_HEADERS})
    list(APPEND sal_headers
      "${PROJECT_SOURCE_DIR}/include/sealtk/${suffix}/${h}")
  endforeach()

  add_library(sealtk_${suffix} ${sal_SOURCES} ${sal_headers})
  add_library(sealtk::${suffix} ALIAS sealtk_${suffix})

  set_target_properties(sealtk_${suffix} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    )

  target_link_libraries(sealtk_${suffix}
    PRIVATE ${sal_PRIVATE_LINK_LIBRARIES}
    PUBLIC ${sal_PUBLIC_LINK_LIBRARIES}
    INTERFACE ${sal_INTERFACE_LINK_LIBRARIES}
    )
endfunction()

function(sealtk_add_executable name)
  sealtk_parse_name(${name} suffix)

  set(sae_multi
    SOURCES
    PRIVATE_LINK_LIBRARIES
    PUBLIC_LINK_LIBRARIES
    INTERFACE_LINK_LIBRARIES
    )
  cmake_parse_arguments(sae "" "" "${sae_multi}" ${ARGN})

  add_executable(sealtk_${suffix} ${sae_SOURCES})
  add_executable(sealtk::${suffix} ALIAS sealtk_${suffix})

  set_target_properties(sealtk_${suffix} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
    )

  target_link_libraries(sealtk_${suffix}
    PRIVATE ${sae_PRIVATE_LINK_LIBRARIES}
    PUBLIC ${sae_PUBLIC_LINK_LIBRARIES}
    INTERFACE ${sae_INTERFACE_LINK_LIBRARIES}
    )
endfunction()
