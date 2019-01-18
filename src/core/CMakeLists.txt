# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

set(SEALTK_CORE_CONFIG_IN
  "${PROJECT_SOURCE_DIR}/include/sealtk/core/Config.h.in")
set(SEALTK_CORE_CONFIG_OUT
  "${PROJECT_BINARY_DIR}/include/sealtk/core/Config.h")
set(SEALTK_CORE_VERSION_IN
  "${PROJECT_SOURCE_DIR}/include/sealtk/core/Version.h.in")
set(SEALTK_CORE_VERSION_OUT
  "${PROJECT_BINARY_DIR}/include/sealtk/core/Version.h")

sealtk_add_library(core
  SOURCES
    VideoSource.cpp
    "${SEALTK_CORE_CONFIG_OUT}"
    "${SEALTK_CORE_VERSION_OUT}"

  HEADERS
    VideoSource.hpp
  )
target_link_libraries(sealtk_core
  Qt5::Core
  qtExtensions
  kwiver::vital
  kwiver::kwiver_algo_qt
  )

configure_file(
  "${SEALTK_CORE_CONFIG_IN}"
  "${SEALTK_CORE_CONFIG_OUT}"
  )

set(SEALTK_VERSION "v${PROJECT_VERSION}")
if(TARGET Git::Git)
  add_custom_command(
    OUTPUT "${SEALTK_CORE_VERSION_OUT}"
    COMMAND "${CMAKE_COMMAND}"
      "-DGIT_EXECUTABLE:FILEPATH=$<TARGET_FILE:Git::Git>"
      "-DSEALTK_SOURCE_DIR:PATH=${PROJECT_SOURCE_DIR}"
      "-DSEALTK_VERSION_FALLBACK:STRING=${SEALTK_VERSION}"
      "-DSEALTK_VERSION_IN:FILEPATH=${SEALTK_CORE_VERSION_IN}"
      "-DSEALTK_VERSION_OUT:FILEPATH=${SEALTK_CORE_VERSION_OUT}"
      -P "${PROJECT_SOURCE_DIR}/cmake/GenerateVersion.cmake"
    )
else()
  configure_file(
    "${SEALTK_CORE_VERSION_IN}"
    "${SEALTK_CORE_VERSION_OUT}"
    )
endif()
