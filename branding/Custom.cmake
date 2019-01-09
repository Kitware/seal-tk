# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include("${CMAKE_CURRENT_LIST_DIR}/SEAL-TK.cmake")

set(cache_params)
macro(cache_param name type description)
  set(SEALTK_CUSTOM_${name} "${SEALTK_${name}}" CACHE ${type} "${description}")
  mark_as_advanced(SEALTK_CUSTOM_${name})
  set(SEALTK_${name} "${SEALTK_CUSTOM_${name}}")
endmacro()

cache_param(EXECUTABLE_NAME STRING "Name of executable")
cache_param(TITLE STRING "Program title")
cache_param(DESCRIPTION STRING "Program description")
cache_param(RESOURCE_FILE FILEPATH "Qt resource file")
