/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/ImageUtils.hpp>

#include <arrows/qt/image_container.h>

namespace sealtk
{

namespace core
{

// ----------------------------------------------------------------------------
QImage imageContainerToQImage(
  kwiver::vital::image_container_sptr const& image)
{
  auto qContainer =
    std::dynamic_pointer_cast<kwiver::arrows::qt::image_container>(image);
  if (qContainer)
  {
    return *qContainer;
  }

  return kwiver::arrows::qt::image_container::vital_to_qt(image->get_image());
}

}

}
