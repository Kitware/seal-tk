/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_ImageUtils_hpp
#define sealtk_core_ImageUtils_hpp

#include <sealtk/core/Export.h>

#include <QImage>

#include <vital/types/image_container.h>

namespace sealtk
{

namespace core
{

QImage SEALTK_CORE_EXPORT imageContainerToQImage(
  kwiver::vital::image_container_sptr const& image);

}

}

#endif
