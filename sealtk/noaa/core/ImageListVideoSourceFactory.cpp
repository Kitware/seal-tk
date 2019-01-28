/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/core/ImageListVideoSourceFactory.hpp>

namespace sealtk
{

namespace noaa
{

namespace core
{

// ============================================================================
class ImageListVideoSourceFactoryPrivate
{
public:
};

// ----------------------------------------------------------------------------
ImageListVideoSourceFactory::ImageListVideoSourceFactory(
  sealtk::core::VideoController* parent)
  : sealtk::core::KwiverFileVideoSourceFactory{parent},
    d_ptr{new ImageListVideoSourceFactoryPrivate}
{
}

// ----------------------------------------------------------------------------
ImageListVideoSourceFactory::~ImageListVideoSourceFactory()
{
}

// ----------------------------------------------------------------------------
kwiver::vital::config_block_sptr ImageListVideoSourceFactory::config(
  QString const& path) const
{
  auto config = kwiver::vital::config_block::empty_config();
  config->set_value("video_reader:type", "image_list");
  config->set_value("video_reader:image_list:image_reader:type",
    "noaa_timestamp_passthrough");
  config->set_value("video_reader:image_list:image_reader:"
    "noaa_timestamp_passthrough:image_reader:type", "qt");

  return config;
}

}

}

}
