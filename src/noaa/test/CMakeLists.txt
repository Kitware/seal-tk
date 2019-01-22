# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(SEALTKUtils)

sealtk_add_test(FilenameUtils
  SOURCES
    FilenameUtils.cpp

  PRIVATE_LINK_LIBRARIES
    sealtk::noaa
  )
