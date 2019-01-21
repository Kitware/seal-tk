# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(SEALTKUtils)

sealtk_add_library(sealtk::core_test_common
  STATIC

  SOURCES
    TestCommon.cpp

  PRIVATE_LINK_LIBRARIES
    Qt5::Core
    kwiver::vital
    kwiver::vital_algo
  )

sealtk_add_test(KwiverVideoSource
  SOURCES
    KwiverVideoSource.cpp

  PRIVATE_LINK_LIBRARIES
    sealtk::core
    sealtk::core_test_common
  )
