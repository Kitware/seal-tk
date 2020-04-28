# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(GenerateExportHeader)
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
    PRIVATE_INCLUDE_DIRECTORIES
    PUBLIC_INCLUDE_DIRECTORIES
    INTERFACE_INCLUDE_DIRECTORIES
    )
  cmake_parse_arguments(sal
    "NOINSTALL"
    "TYPE;TARGET_NAME_VAR;EXPORT_HEADER"
    "${sal_multi}"
    ${ARGN}
    )

  if (sal_TYPE STREQUAL INTERFACE)
    add_library(${suffix} INTERFACE)

    target_link_libraries(${suffix}
      INTERFACE ${sal_PUBLIC_LINK_LIBRARIES}
      )

    target_include_directories(${suffix}
      INTERFACE
        ${sal_PUBLIC_INCLUDE_DIRECTORIES}
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR};${PROJECT_BINARY_DIR}>"
        "$<INSTALL_INTERFACE:include>"
      )
  else()
    add_library(${suffix} ${sal_TYPE} ${sal_SOURCES} ${sal_HEADERS})

    set_target_properties(${suffix} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}"
      LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}"
      ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}"
      OUTPUT_NAME "sealtk_${suffix}"
      )

    target_link_libraries(${suffix}
      PRIVATE ${sal_PRIVATE_LINK_LIBRARIES}
      PUBLIC ${sal_PUBLIC_LINK_LIBRARIES}
      )

    target_include_directories(${suffix}
      PRIVATE
        ${sal_PRIVATE_INCLUDE_DIRECTORIES}
      PUBLIC
        ${sal_PUBLIC_INCLUDE_DIRECTORIES}
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR};${PROJECT_BINARY_DIR}>"
        "$<INSTALL_INTERFACE:include>"
      )
  endif()

  target_link_libraries(${suffix}
    INTERFACE ${sal_INTERFACE_LINK_LIBRARIES}
    )

  target_include_directories(${suffix}
    INTERFACE
      ${sal_INTERFACE_INCLUDE_DIRECTORIES}
    )

  add_library(sealtk::${suffix} ALIAS ${suffix})

  if(sal_EXPORT_HEADER)
    if(NOT IS_ABSOLUTE "${sal_EXPORT_HEADER}")
      set(sal_EXPORT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${sal_EXPORT_HEADER}")
    endif()
    generate_export_header(${suffix}
      BASE_NAME sealtk_${suffix}
      EXPORT_FILE_NAME "${sal_EXPORT_HEADER}"
      )
  endif()

  if(NOT sal_NOINSTALL)
    file(RELATIVE_PATH include_dir
      "${PROJECT_SOURCE_DIR}"
      "${CMAKE_CURRENT_SOURCE_DIR}"
      )
    install(TARGETS ${suffix} EXPORT sealtk
      RUNTIME COMPONENT Runtime DESTINATION "${CMAKE_INSTALL_BINDIR}"
      LIBRARY COMPONENT Runtime DESTINATION "${CMAKE_INSTALL_LIBDIR}"
      ARCHIVE COMPONENT Development DESTINATION "${CMAKE_INSTALL_LIBDIR}"
      )
    install(FILES ${sal_HEADERS} ${sal_EXPORT_HEADER}
      COMPONENT Development DESTINATION
        "${CMAKE_INSTALL_INCLUDEDIR}/${include_dir}"
      )
  endif()

  if(DEFINED sal_TARGET_NAME_VAR)
    set(${sal_TARGET_NAME_VAR} ${suffix} PARENT_SCOPE)
  endif()
endfunction()

function(sealtk_add_kwiver_plugin name)
  sealtk_parse_name(${name} suffix)

  set(sakp_multi
    SOURCES
    HEADERS
    PRIVATE_LINK_LIBRARIES
    PUBLIC_LINK_LIBRARIES
    INTERFACE_LINK_LIBRARIES
    PRIVATE_INCLUDE_DIRECTORIES
    PUBLIC_INCLUDE_DIRECTORIES
    INTERFACE_INCLUDE_DIRECTORIES
    )
  cmake_parse_arguments(sakp
    "NOINSTALL"
    "TARGET_NAME_VAR;EXPORT_HEADER"
    "${sakp_multi}"
    ${ARGN}
    )

  add_library(${suffix} MODULE ${sakp_SOURCES} ${sakp_HEADERS})
  add_library(sealtk::${suffix} ALIAS ${suffix})

  set_target_properties(${suffix} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
      "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/kwiver/modules"
    LIBRARY_OUTPUT_DIRECTORY
      "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/kwiver/modules"
    OUTPUT_NAME "sealtk_${suffix}"
    PREFIX ""
    )

  target_link_libraries(${suffix}
    PRIVATE ${sakp_PRIVATE_LINK_LIBRARIES}
    PUBLIC ${sakp_PUBLIC_LINK_LIBRARIES}
    INTERFACE ${sakp_INTERFACE_LINK_LIBRARIES}
    )

  target_include_directories(${suffix}
    PRIVATE
      ${sakp_PRIVATE_INCLUDE_DIRECTORIES}
    PUBLIC
      ${sakp_PUBLIC_INCLUDE_DIRECTORIES}
      "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR};${PROJECT_BINARY_DIR}>"
      "$<INSTALL_INTERFACE:include>"
    INTERFACE
      ${sakp_INTERFACE_INCLUDE_DIRECTORIES}
    )

  if(sakp_EXPORT_HEADER)
    if(NOT IS_ABSOLUTE "${sakp_EXPORT_HEADER}")
      set(sakp_EXPORT_HEADER
        "${CMAKE_CURRENT_BINARY_DIR}/${sakp_EXPORT_HEADER}"
        )
    endif()
    generate_export_header(${suffix}
      BASE_NAME sealtk_${suffix}
      EXPORT_FILE_NAME "${sakp_EXPORT_HEADER}"
      )
  endif()

  if(NOT sakp_NOINSTALL)
    file(RELATIVE_PATH include_dir
      "${PROJECT_SOURCE_DIR}"
      "${CMAKE_CURRENT_SOURCE_DIR}"
      )
    install(TARGETS ${suffix} EXPORT sealtk
      RUNTIME COMPONENT Runtime DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/kwiver/modules"
      LIBRARY COMPONENT Runtime DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/kwiver/modules"
      )
    install(FILES ${sakp_HEADERS} ${sakp_EXPORT_HEADER}
      COMPONENT Development DESTINATION
        "${CMAKE_INSTALL_INCLUDEDIR}/${include_dir}"
      )
  endif()

  if(DEFINED sakp_TARGET_NAME_VAR)
    set(${sakp_TARGET_NAME_VAR} ${suffix} PARENT_SCOPE)
  endif()
endfunction()

function(sealtk_add_executable name)
  sealtk_parse_name(${name} suffix)

  set(sae_multi
    SOURCES
    PRIVATE_LINK_LIBRARIES
    PUBLIC_LINK_LIBRARIES
    INTERFACE_LINK_LIBRARIES
    PRIVATE_INCLUDE_DIRECTORIES
    PUBLIC_INCLUDE_DIRECTORIES
    INTERFACE_INCLUDE_DIRECTORIES
    )
  cmake_parse_arguments(sae
    "NOINSTALL"
    "TARGET_NAME_VAR"
    "${sae_multi}"
    ${ARGN}
    )

  add_executable(${suffix} ${sae_SOURCES})
  add_executable(sealtk::${suffix} ALIAS ${suffix})

  set_target_properties(${suffix} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}"
    )

  target_link_libraries(${suffix}
    PRIVATE ${sae_PRIVATE_LINK_LIBRARIES}
    PUBLIC ${sae_PUBLIC_LINK_LIBRARIES}
    INTERFACE ${sae_INTERFACE_LINK_LIBRARIES}
    )

  target_include_directories(${suffix}
    PRIVATE
      ${sae_PRIVATE_INCLUDE_DIRECTORIES}
    PUBLIC
      ${sae_PUBLIC_INCLUDE_DIRECTORIES}
      "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR};${PROJECT_BINARY_DIR}>"
      "$<INSTALL_INTERFACE:include>"
    INTERFACE
      ${sae_INTERFACE_INCLUDE_DIRECTORIES}
    )

  if(NOT sae_NOINSTALL)
    install(TARGETS ${suffix} EXPORT sealtk
      RUNTIME COMPONENT Runtime DESTINATION "${CMAKE_INSTALL_BINDIR}"
      )
  endif()

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
    PRIVATE_INCLUDE_DIRECTORIES
    PUBLIC_INCLUDE_DIRECTORIES
    INTERFACE_INCLUDE_DIRECTORIES
    )
  cmake_parse_arguments(sat "" "" "${sat_multi}" ${ARGN})

  add_executable(test_${name} ${sat_SOURCES})

  target_link_libraries(test_${name}
    PRIVATE ${sat_PRIVATE_LINK_LIBRARIES} Qt5::Test
    PUBLIC ${sat_PUBLIC_LINK_LIBRARIES}
    INTERFACE ${sat_INTERFACE_LINK_LIBRARIES}
    )

  target_include_directories(test_${name}
    PRIVATE
      ${sat_PRIVATE_INCLUDE_DIRECTORIES}
    PUBLIC
      ${sat_PUBLIC_INCLUDE_DIRECTORIES}
      "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR};${PROJECT_BINARY_DIR}>"
      "$<INSTALL_INTERFACE:include>"
    INTERFACE
      ${sat_INTERFACE_INCLUDE_DIRECTORIES}
    )

  target_compile_definitions(test_${name}
    PRIVATE "SEALTK_TEST_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data\""
    )

  qt5_discover_tests(${name} test_${name})
endfunction()

function(sealtk_add_data target)
  cmake_parse_arguments(sad "" "FROM" "FILES" ${ARGN})

  if (NOT sad_FROM STREQUAL "")
    set(from "${sad_FROM}/")
  else()
    set(from "")
  endif()

  set(all_files "")
  foreach(file IN LISTS sad_FILES)
    get_filename_component(dir "${file}" DIRECTORY)

    if (IS_ABSOLUTE "${from}")
      set(in "${from}${file}")
    else()
      set(in "${CMAKE_CURRENT_SOURCE_DIR}/${from}${file}")
    endif()
    set(out "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_DATADIR}/${file}")
    set(outdir "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_DATADIR}/${dir}")

    add_custom_command(
      OUTPUT "${out}"
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${outdir}"
      COMMAND "${CMAKE_COMMAND}" -E copy "${in}" "${out}"
      DEPENDS "${in}"
      )
    list(APPEND all_files "${out}")

    install(
      FILES "${from}${file}"
      DESTINATION "${CMAKE_INSTALL_DATADIR}/${dir}")
  endforeach()

  add_custom_target(${target} DEPENDS ${all_files})
endfunction()

# Clean out old data files; this will cause slightly more work after a
# reconfigure, but ensures that no stale files will linger when something is
# removed from the source tree
file(REMOVE_RECURSE "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_DATADIR}")
