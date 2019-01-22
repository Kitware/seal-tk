# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(SEALTKUtils)

set(SEALTK_CORE_VERSION_IN
  "${PROJECT_SOURCE_DIR}/include/sealtk/core/Version.h.in")
set(SEALTK_CORE_VERSION_OUT
  "${PROJECT_BINARY_DIR}/include/sealtk/core/Version.h")

sealtk_add_library(sealtk::core
  SOURCES
    DateUtils.cpp
    FileVideoSourceFactory.cpp
    KwiverFileVideoSourceFactory.cpp
    KwiverVideoSource.cpp
    VideoController.cpp
    VideoSource.cpp
    VideoSourceFactory.cpp
    "${SEALTK_CORE_VERSION_OUT}"

  HEADERS
    DateUtils.hpp
    FileVideoSourceFactory.hpp
    KwiverFileVideoSourceFactory.hpp
    KwiverVideoSource.hpp
    TimeMap.hpp
    VideoController.hpp
    VideoSource.hpp
    VideoSourceFactory.hpp

  PUBLIC_LINK_LIBRARIES
    Qt5::Core
    qtExtensions
    kwiver::vital
    kwiver::kwiver_algo_qt

  TARGET_NAME_VAR name
  )

set(SEALTK_VERSION "v${PROJECT_VERSION}")
if(TARGET Git::Git)
  add_custom_target(sealtk_core_version
    COMMAND "${CMAKE_COMMAND}"
      "-DGIT_EXECUTABLE:FILEPATH=$<TARGET_FILE:Git::Git>"
      "-DSEALTK_SOURCE_DIR:PATH=${PROJECT_SOURCE_DIR}"
      "-DSEALTK_VERSION_FALLBACK:STRING=${SEALTK_VERSION}"
      "-DSEALTK_VERSION_IN:FILEPATH=${SEALTK_CORE_VERSION_IN}"
      "-DSEALTK_VERSION_OUT:FILEPATH=${SEALTK_CORE_VERSION_OUT}"
      -P "${PROJECT_SOURCE_DIR}/cmake/GenerateVersion.cmake"
    BYPRODUCTS "${SEALTK_CORE_VERSION_OUT}"
    )
  add_dependencies(${name} sealtk_core_version)
else()
  configure_file(
    "${SEALTK_CORE_VERSION_IN}"
    "${SEALTK_CORE_VERSION_OUT}"
    )
endif()

add_subdirectory(test)
