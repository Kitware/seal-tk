# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(SEALTKUtils)

sealtk_add_library(sealtk::core_test_common
  TYPE STATIC
  NOINSTALL

  SOURCES
    TestCommon.cpp
    TestVideo.cpp

  PRIVATE_LINK_LIBRARIES
    qtExtensionsHeaders
    Qt5::Core
    sealtk::core
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

sealtk_add_test(VideoController
  SOURCES
    VideoController.cpp

  PRIVATE_LINK_LIBRARIES
    sealtk::core
    sealtk::core_test_common
  )

sealtk_add_test(ImageUtils
  SOURCES
    ImageUtils.cpp

  PRIVATE_LINK_LIBRARIES
    sealtk::core
  )

sealtk_add_test(AutoLevelsTask
  SOURCES
    AutoLevelsTask.cpp

  PRIVATE_LINK_LIBRARIES
    sealtk::core
  )

sealtk_add_test(DateUtils
  SOURCES
    DateUtils.cpp

  PRIVATE_LINK_LIBRARIES
    sealtk::core
  )

sealtk_add_test(TimeMap
  SOURCES
    TimeMap.cpp

  PRIVATE_LINK_LIBRARIES
    sealtk::core
  )

sealtk_add_test(TimeStamp
  SOURCES
    TimeStamp.cpp

  PRIVATE_LINK_LIBRARIES
    sealtk::core
  )
