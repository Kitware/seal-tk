# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

include(SEALTKUtils)

sealtk_add_test(AbstractItemRepresentation
  SOURCES
    AbstractItemRepresentation.cpp

  PRIVATE_LINK_LIBRARIES
    sealtk::gui
    sealtk::core
  )

sealtk_add_test(FusionModel
  SOURCES
    FusionModel.cpp

  PRIVATE_LINK_LIBRARIES
    sealtk::gui
    sealtk::core
  )
