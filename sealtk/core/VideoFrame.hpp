/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_VideoFrame_hpp
#define sealtk_core_VideoFrame_hpp

#include <sealtk/core/VideoMetaData.hpp>

#include <vital/types/image_container.h>

namespace sealtk
{

namespace core
{

// ============================================================================
struct VideoFrame
{
  kwiver::vital::image_container_sptr image;
  VideoMetaData metaData;
};

} // namespace core

} // namespace sealtk

#endif
