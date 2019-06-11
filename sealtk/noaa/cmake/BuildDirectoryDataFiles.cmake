# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

file(REMOVE_RECURSE "${DATA_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/../share/seal-tk" DESTINATION
  "${DATA_BINARY_DIR}"
  )
