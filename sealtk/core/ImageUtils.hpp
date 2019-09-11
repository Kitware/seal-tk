/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_ImageUtils_hpp
#define sealtk_core_ImageUtils_hpp

#include <sealtk/core/Export.h>

#include <vital/types/image_container.h>

class QImage;
class QOpenGLTexture;

namespace sealtk
{

namespace core
{

QImage SEALTK_CORE_EXPORT imageContainerToQImage(
  kwiver::vital::image_container_sptr const& image);

void SEALTK_CORE_EXPORT imageToTexture(
  QOpenGLTexture& texture,
  kwiver::vital::image_container_sptr const& image);

} // namespace core

} // namespace sealtk

#endif
