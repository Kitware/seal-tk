# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

add_library(SEALTKLib INTERFACE)
target_include_directories(SEALTKLib INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  )

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/Config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/Config.h"
  )

set(SEALTK_VERSION "v${PROJECT_VERSION}")
if(TARGET Git::Git)
  add_custom_target(
    SEALTKLibVersion
    COMMAND "${CMAKE_COMMAND}"
      "-DGIT_EXECUTABLE:FILEPATH=$<TARGET_FILE:Git::Git>"
      "-DSEALTK_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}"
      "-DSEALTK_VERSION_FALLBACK:STRING=${SEALTK_VERSION}"
      "-DSEALTK_VERSION_IN:FILEPATH=${CMAKE_CURRENT_SOURCE_DIR}/Version.h.in"
      "-DSEALTK_VERSION_OUT:FILEPATH=${CMAKE_CURRENT_BINARY_DIR}/Version.h"
      -P "${CMAKE_SOURCE_DIR}/cmake/GenerateVersion.cmake"
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/Version.h"
    )
  add_dependencies(SEALTKLib SEALTKLibVersion)
else()
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Version.h"
    )
endif()
