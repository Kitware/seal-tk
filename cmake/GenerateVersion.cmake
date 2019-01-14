# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

execute_process(
  COMMAND "${GIT_EXECUTABLE}" describe --dirty --always
  OUTPUT_VARIABLE SEALTK_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
  WORKING_DIRECTORY "${SEALTK_SOURCE_DIR}"
  )

if(NOT SEALTK_VERSION)
  set(SEALTK_VERSION "${SEALTK_VERSION_FALLBACK}")
endif()

configure_file(
  "${SEALTK_VERSION_IN}"
  "${SEALTK_VERSION_OUT}"
  )
