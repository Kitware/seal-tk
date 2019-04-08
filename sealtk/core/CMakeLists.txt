# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(SEALTKUtils)

set(SEALTK_CORE_VERSION_IN
  "${PROJECT_SOURCE_DIR}/sealtk/core/Version.h.in")
set(SEALTK_CORE_VERSION_OUT
  "${PROJECT_BINARY_DIR}/sealtk/core/Version.h")

sealtk_add_library(sealtk::core
  SOURCES
    DateUtils.cpp
    FileVideoSourceFactory.cpp
    ImageUtils.cpp
    KwiverFileVideoSourceFactory.cpp
    KwiverVideoSource.cpp
    TimeStamp.cpp
    VideoController.cpp
    VideoMetaData.cpp
    VideoSource.cpp
    VideoSourceFactory.cpp

  HEADERS
    DateUtils.hpp
    FileVideoSourceFactory.hpp
    ImageUtils.hpp
    KwiverFileVideoSourceFactory.hpp
    KwiverVideoSource.hpp
    TimeMap.hpp
    TimeStamp.hpp
    VideoController.hpp
    VideoMetaData.hpp
    VideoSource.hpp
    VideoSourceFactory.hpp
    "${SEALTK_CORE_VERSION_OUT}"

  PUBLIC_LINK_LIBRARIES
    Qt5::Core
    qtExtensions
    kwiver::vital
    kwiver::kwiver_algo_qt

  PUBLIC_INCLUDE_DIRECTORIES
    "${EIGEN3_INCLUDE_DIR}"

  EXPORT_HEADER Export.h
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
