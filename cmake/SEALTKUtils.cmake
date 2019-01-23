# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(GNUInstallDirs)

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
    message(FATAL_ERROR "Target name must be of the form "
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
  cmake_parse_arguments(sal
    "STATIC;SHARED"
    "TARGET_NAME_VAR"
    "${sal_multi}"
    ${ARGN}
    )

  if(sal_STATIC AND sal_SHARED)
    message(FATAL_ERROR "Cannot have a library that is both STATIC and SHARED")
  elseif(sal_STATIC)
    set(type STATIC)
  elseif(sal_SHARED)
    set(type SHARED)
  else()
    set(type)
  endif()

  add_library(${suffix} ${type} ${sal_SOURCES} ${sal_HEADERS})
  add_library(sealtk::${suffix} ALIAS ${suffix})

  set_target_properties(${suffix} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    OUTPUT_NAME "sealtk_${suffix}"
    )

  target_link_libraries(${suffix}
    PRIVATE ${sal_PRIVATE_LINK_LIBRARIES}
    PUBLIC ${sal_PUBLIC_LINK_LIBRARIES}
    INTERFACE ${sal_INTERFACE_LINK_LIBRARIES}
    )

  file(RELATIVE_PATH include_dir
    "${PROJECT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    )

  install(TARGETS ${suffix}
    RUNTIME COMPONENT Runtime DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY COMPONENT Runtime DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE COMPONENT Development DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    )
  install(FILES ${sal_HEADERS}
    COMPONENT Development DESTINATION
      "${CMAKE_INSTALL_INCLUDEDIR}/${include_dir}"
    )

  if(DEFINED sal_TARGET_NAME_VAR)
    set(${sal_TARGET_NAME_VAR} ${suffix} PARENT_SCOPE)
  endif()
endfunction()

function(sealtk_add_executable name)
  sealtk_parse_name(${name} suffix)

  set(sae_multi
    SOURCES
    PRIVATE_LINK_LIBRARIES
    PUBLIC_LINK_LIBRARIES
    INTERFACE_LINK_LIBRARIES
    )
  cmake_parse_arguments(sae "" "TARGET_NAME_VAR" "${sae_multi}" ${ARGN})

  add_executable(${suffix} ${sae_SOURCES})
  add_executable(sealtk::${suffix} ALIAS ${suffix})

  set_target_properties(${suffix} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
    )

  target_link_libraries(${suffix}
    PRIVATE ${sae_PRIVATE_LINK_LIBRARIES}
    PUBLIC ${sae_PUBLIC_LINK_LIBRARIES}
    INTERFACE ${sae_INTERFACE_LINK_LIBRARIES}
    )

  install(TARGETS ${suffix}
    RUNTIME COMPONENT Runtime DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )

  if(DEFINED sae_TARGET_NAME_VAR)
    set(${sae_TARGET_NAME_VAR} ${suffix} PARENT_SCOPE)
  endif()
endfunction()

function(sealtk_add_test name)
  set(sat_multi
    SOURCES
    PRIVATE_LINK_LIBRARIES
    PUBLIC_LINK_LIBRARIES
    INTERFACE_LINK_LIBRARIES
    )
  cmake_parse_arguments(sat "" "" "${sat_multi}" ${ARGN})

  add_executable(test_${name} ${sat_SOURCES})

  target_link_libraries(test_${name}
    PRIVATE ${sat_PRIVATE_LINK_LIBRARIES} Qt5::Test
    PUBLIC ${saj_PUBLIC_LINK_LIBRARIES}
    INTERFACE ${sat_INTERFACE_LINK_LIBRARIES}
    )
  target_compile_definitions(test_${name}
    PRIVATE "SEALTK_TEST_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data\""
    )
  qt5_discover_tests(${name} test_${name})
endfunction()
